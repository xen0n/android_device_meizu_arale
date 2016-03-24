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

#define LOG_TAG "MtkCam/HalSensor"
//
#include "MyUtils.h"
#include "sensor_drv.h"
#ifdef USING_MTK_LDVT
#include "uvvf.h"
#endif
#include "mtkcam/common.h"
#include <mtkcam/exif/IBaseCamExif.h>
#include <debug_exif/dbg_id_param.h>
#include <debug_exif/cam/dbg_cam_param.h>
// For property_get().
#include <cutils/properties.h>


#define TWO_PIXEL_MODE_THRESHOLD (3000000)// 16MP @15fps + 5MP @15fps
//
extern SENSORDRV_INFO_STRUCT sensorDrvInfo[3];
extern SENSOR_HAL_RAW_INFO_STRUCT sensorRawInfo[3];

extern MUINT32 meSensorDev;
extern MINT32 mPowerRefCount;

extern SENSOR_DRIVER_SCENARIO_T g_ScenarioconfMain; /*LukeHu++150410=For ESD*/
extern SENSOR_DRIVER_SCENARIO_T g_ScenarioconfSub;	/*LukeHu++150410=For ESD*/
extern SENSOR_DRIVER_SCENARIO_T g_ScenarioconfMain2;/*LukeHu++150410=For ESD*/
//
SENSOR_CONFIG_STRUCT sensorPara[3];
SensorDynamicInfo sensorDynamicInfo[3];
SENSOR_HAL_TEST_MODEL_STRUCT sensorTMPara;
SENSOR_VC_INFO_STRUCT gVCInfo;

//hwsync
//#include "hwsync_drv.h"
//extern HWSyncDrv* mpHwSyncDrv;

/******************************************************************************
 *
 ******************************************************************************/
#ifndef USING_MTK_LDVT
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        VV_MSG("[%s] " fmt, __FUNCTION__, ##arg)
#endif

//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


#define ROUND_TO_2X(x) ((x) & (~0x1))

/******************************************************************************
 *
 ******************************************************************************/
MBOOL HalSensor::querySensorDynamicInfo(
   MUINT32 sensorIdx,
   SensorDynamicInfo *pSensorDynamicInfo
)
{
   MBOOL ret = MFALSE;
    switch (sensorIdx) {
        case SENSOR_DEV_MAIN:
            memcpy(pSensorDynamicInfo, &sensorDynamicInfo[0],sizeof(SensorDynamicInfo));
            break;
        case SENSOR_DEV_SUB:
            memcpy(pSensorDynamicInfo, &sensorDynamicInfo[1],sizeof(SensorDynamicInfo));
            break;
        case SENSOR_DEV_MAIN_2:
            memcpy(pSensorDynamicInfo, &sensorDynamicInfo[2],sizeof(SensorDynamicInfo));
            break;
        default:
            return ret;
            break;

    }
    ret = MTRUE;
    return ret;
}

MBOOL HalSensor::configure(
    MUINT const         uCountOfParam,
    ConfigParam const*  pArrayOfParam
)
{
    MBOOL ret = MFALSE;

    MINT32 pixelX0 = 0, pixelY0 = 0;
    MINT32 srcWidth = 0,srcHeight = 0;
    MUINT sensorScenarioId;
    MUINT32 inDataFmt = 0,data1 = 0, data2 = 0;
    SENSOR_DEV_ENUM eSensorDev=SENSOR_NONE;
    MUINT32 currSensorDev=0;
    MUINT32 currFPS=0;
    MINT idx=0;
    ConfigParam halSensorIFParam[3];
    SENSOR_VC_INFO_STRUCT VcInfo;
    SensorDrv *const pSensorDrv = SensorDrv::get();
    SENSOR_DRIVER_SCENARIO_T scenarioconf;
#ifndef USING_MTK_LDVT
        //CPTLog(Event_Sensor_setScenario, CPTFlagStart);
#endif


    Mutex::Autolock _l(mMutex);
    //

    //get property for hwsync
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.hwsync.enable", value, "0");
    int hwsyncEnable=atoi(value);

    // get property for ef test
    char value2[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.ef.test", value2, "0");
    int efTestEnable=atoi(value2);
    //

    if  ( 0 == mPowerRefCount )
    {
        MY_LOGW("Should powerOn before configure !!");
        goto lbExit;
    }



    for (MUINT i = 0; i < uCountOfParam; i++)
    {
        //  pArrayOfParam[i].index -> eSensorDev
        MUINT const sensorDev = HalSensorList::singleton()->queryEnumInfoByIndex(pArrayOfParam[i].index)->meSensorDev;

        switch(sensorDev) {
            case SENSOR_DEV_MAIN:
                idx = 0;
                break;
            case SENSOR_DEV_SUB:
                idx = 1;
                break;
            case SENSOR_DEV_MAIN_2:
                idx = 2;
                break;
            default:
                idx = 0;
                break;
        }

        halSensorIFParam[idx].crop              = pArrayOfParam[i].crop;
        halSensorIFParam[idx].scenarioId        = pArrayOfParam[i].scenarioId;
        halSensorIFParam[idx].isBypassScenario  = pArrayOfParam[i].isBypassScenario;
        halSensorIFParam[idx].isContinuous      = pArrayOfParam[i].isContinuous;
        halSensorIFParam[idx].HDRMode           = pArrayOfParam[i].HDRMode; // 0: no, 1: IHDR, 2:Merge mode HDR
        halSensorIFParam[idx].framerate         = pArrayOfParam[i].framerate*10; // Unit : FPS , Driver Unit : 10*FPS
        halSensorIFParam[idx].twopixelOn        = pArrayOfParam[i].twopixelOn;
        halSensorIFParam[idx].debugMode         = pArrayOfParam[i].debugMode;

        currSensorDev |= sensorDev;
    }

    currFPS = halSensorIFParam[idx].framerate;
    //Test Mode use property parameter
    {
        property_get("debug.senif.hdrmode", value, "0");
        int hdrModeTest=atoi(value);
        if((hdrModeTest == 1)||(hdrModeTest == 2)||(hdrModeTest == 9))
        {
            halSensorIFParam[idx].HDRMode = hdrModeTest;
        }
    }

    MY_LOGD("SenDev=%d, scenario=%d, HDR=%d, fps=%d, twopix=%d\n",currSensorDev, halSensorIFParam[idx].scenarioId,
        halSensorIFParam[idx].HDRMode,halSensorIFParam[idx].framerate,halSensorIFParam[idx].twopixelOn);

    // TM setting for debug mode 1
    sensorTMPara.TM_Vsync       = 16;
    sensorTMPara.TM_DummyPixel  = 16;
    sensorTMPara.TM_Line        = 4500;
    sensorTMPara.TM_Pixel       = 6500;
    sensorTMPara.TM_PAT         = 0;
    sensorTMPara.TM_FMT         = 0;


    if(currSensorDev & SENSOR_DEV_MAIN ) {

        if(halSensorIFParam[0].isBypassScenario != 0) {
            goto lbExit;
        }
        if(halSensorIFParam[idx].framerate == 0)
        {
            data1 = halSensorIFParam[0].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
            halSensorIFParam[idx].framerate = data2;
            currFPS = halSensorIFParam[idx].framerate;
            MY_LOGD("configure Scenario=%d, FPS=%d\n",data1,data2);
        }
        if(halSensorIFParam[idx].HDRMode == 2)
        {
            data1 = halSensorIFParam[0].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_SENSOR_VC_INFO, (MUINTPTR)&VcInfo, (MUINTPTR)&data1);
            memcpy((void *)&gVCInfo, (void *)&VcInfo, sizeof(SENSOR_VC_INFO_STRUCT));
        }
        switch(halSensorIFParam[0].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                sensorPara[0].u1MIPIDataSettleDelay=sensorDrvInfo[0].MIPIDataLowPwr2HSSettleDelayM0;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_PRE;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                sensorPara[0].u1MIPIDataSettleDelay=sensorDrvInfo[0].MIPIDataLowPwr2HSSettleDelayM1;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_CAP;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                sensorPara[0].u1MIPIDataSettleDelay=sensorDrvInfo[0].MIPIDataLowPwr2HSSettleDelayM2;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_VD;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                sensorPara[0].u1MIPIDataSettleDelay=sensorDrvInfo[0].MIPIDataLowPwr2HSSettleDelayM3;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_VD1;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                sensorPara[0].u1MIPIDataSettleDelay=sensorDrvInfo[0].MIPIDataLowPwr2HSSettleDelayM4;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_VD2;
                break;
            default:
                sensorPara[0].u1MIPIDataSettleDelay = sensorDrvInfo[0].MIPIDataLowPwr2HighSpeedSettleDelayCount;
                sensorPara[0].DPCM_TYPE=sensorDrvInfo[0].IMGSENSOR_DPCM_TYPE_PRE;
                break;
        }

        sensorPara[0].scenarioId = halSensorIFParam[0].scenarioId;
        sensorPara[0].u1IsBypassSensorScenario = (MUINT8)halSensorIFParam[0].isBypassScenario;
        sensorPara[0].u1IsContinuous = (MUINT8)halSensorIFParam[0].isContinuous;
        sensorPara[0].u1MIPIDataTermDelay = sensorDrvInfo[0].MIPIDataLowPwr2HighSpeedTermDelayCount;
        //sensorPara[0].u1MIPIDataSettleDelay = sensorDrvInfo[0].MIPIDataLowPwr2HighSpeedSettleDelayCount;
        sensorPara[0].u1MIPIClkTermDelay = sensorDrvInfo[0].MIPICLKLowPwr2HighSpeedTermDelayCount;
        sensorPara[0].u1MIPILaneNum = sensorDrvInfo[0].SensorMIPILaneNumber;
        sensorPara[0].u1MIPIPacketECCOrder = sensorDrvInfo[0].SensorPacketECCOrder;
        sensorPara[0].MIPI_OPHY_TYPE = (MUINT)sensorDrvInfo[0].MIPIsensorType;
        sensorPara[0].NCSI2_HSRXDET_MODE = (MUINT)sensorDrvInfo[0].SettleDelayMode;
        sensorPara[0].cropWidth = halSensorIFParam[idx].crop.w;
        sensorPara[0].cropHeight = halSensorIFParam[idx].crop.h;
        sensorPara[0].frameRate = halSensorIFParam[idx].framerate;
        sensorPara[0].twopixelOn = halSensorIFParam[idx].twopixelOn;
        sensorPara[0].debugMode = halSensorIFParam[idx].debugMode;
        sensorPara[0].HDRMode = halSensorIFParam[idx].HDRMode;
        sensorPara[0].SCAM_DataNumber = (MUINT)sensorDrvInfo[0].SCAM_DataNumber;
        sensorPara[0].SCAM_DDR_En = (MUINT)sensorDrvInfo[0].SCAM_DDR_En;
        sensorPara[0].SCAM_CLK_INV = (MUINT)sensorDrvInfo[0].SCAM_CLK_INV;
     }


    if(currSensorDev & SENSOR_DEV_SUB ) {
        if(halSensorIFParam[1].isBypassScenario != 0) {
            goto lbExit;
        }

        if(halSensorIFParam[idx].framerate == 0)
        {
            data1 = halSensorIFParam[1].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
            halSensorIFParam[idx].framerate = data2;
            currFPS = halSensorIFParam[idx].framerate;
            MY_LOGD("configure Scenario=%d, FPS=%d\n",data1,data2);
        }

        switch(halSensorIFParam[1].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                sensorPara[1].u1MIPIDataSettleDelay=sensorDrvInfo[1].MIPIDataLowPwr2HSSettleDelayM0;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_PRE;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                sensorPara[1].u1MIPIDataSettleDelay=sensorDrvInfo[1].MIPIDataLowPwr2HSSettleDelayM1;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_CAP;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                sensorPara[1].u1MIPIDataSettleDelay=sensorDrvInfo[1].MIPIDataLowPwr2HSSettleDelayM2;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_VD;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                sensorPara[1].u1MIPIDataSettleDelay=sensorDrvInfo[1].MIPIDataLowPwr2HSSettleDelayM3;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_VD1;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                sensorPara[1].u1MIPIDataSettleDelay=sensorDrvInfo[1].MIPIDataLowPwr2HSSettleDelayM4;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_VD2;
                break;
            default:
                sensorPara[1].u1MIPIDataSettleDelay = sensorDrvInfo[1].MIPIDataLowPwr2HighSpeedSettleDelayCount;
                sensorPara[1].DPCM_TYPE=sensorDrvInfo[1].IMGSENSOR_DPCM_TYPE_PRE;
                break;
        }
        sensorPara[1].scenarioId = halSensorIFParam[1].scenarioId;
        sensorPara[1].u1IsBypassSensorScenario = (MUINT8)halSensorIFParam[1].isBypassScenario;
        sensorPara[1].u1IsContinuous = (MUINT8)halSensorIFParam[1].isContinuous;
        sensorPara[1].u1MIPIDataTermDelay = sensorDrvInfo[1].MIPIDataLowPwr2HighSpeedTermDelayCount;
        //sensorPara[1].u1MIPIDataSettleDelay = sensorDrvInfo[1].MIPIDataLowPwr2HighSpeedSettleDelayCount;
        sensorPara[1].u1MIPIClkTermDelay = sensorDrvInfo[1].MIPICLKLowPwr2HighSpeedTermDelayCount;
        sensorPara[1].u1MIPILaneNum = sensorDrvInfo[1].SensorMIPILaneNumber;
        sensorPara[1].u1MIPIPacketECCOrder = sensorDrvInfo[1].SensorPacketECCOrder;
        sensorPara[1].MIPI_OPHY_TYPE = (MUINT)sensorDrvInfo[1].MIPIsensorType;
        sensorPara[1].NCSI2_HSRXDET_MODE = (MUINT)sensorDrvInfo[1].SettleDelayMode;
        sensorPara[1].cropWidth = halSensorIFParam[idx].crop.w;
        sensorPara[1].cropHeight = halSensorIFParam[idx].crop.h;
        sensorPara[1].frameRate = halSensorIFParam[idx].framerate;
        sensorPara[1].twopixelOn = halSensorIFParam[idx].twopixelOn;
        sensorPara[1].debugMode = halSensorIFParam[idx].debugMode;
        sensorPara[1].HDRMode = halSensorIFParam[idx].HDRMode;
        sensorPara[1].SCAM_DataNumber = (MUINT)sensorDrvInfo[1].SCAM_DataNumber;
        sensorPara[1].SCAM_DDR_En = (MUINT)sensorDrvInfo[1].SCAM_DDR_En;
        sensorPara[1].SCAM_CLK_INV = (MUINT)sensorDrvInfo[1].SCAM_CLK_INV;
    }

    if(currSensorDev & SENSOR_DEV_MAIN_2 ) {
        if(halSensorIFParam[2].isBypassScenario != 0) {
            goto lbExit;
        }

        if(halSensorIFParam[idx].framerate == 0)
        {
            data1 = halSensorIFParam[2].scenarioId;
            ret = pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
            halSensorIFParam[idx].framerate = data2;
            currFPS = halSensorIFParam[idx].framerate;
            MY_LOGD("configure Scenario=%d, FPS=%d\n",data1,data2);
        }
        switch(halSensorIFParam[2].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HSSettleDelayM0;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_PRE;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HSSettleDelayM1;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_CAP;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HSSettleDelayM2;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_VD;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HSSettleDelayM3;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_VD1;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HSSettleDelayM4;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_VD2;
                break;
            default:
                sensorPara[2].u1MIPIDataSettleDelay=sensorDrvInfo[2].MIPIDataLowPwr2HighSpeedSettleDelayCount;
                sensorPara[2].DPCM_TYPE=sensorDrvInfo[2].IMGSENSOR_DPCM_TYPE_PRE;
                break;
        }
        sensorPara[2].scenarioId = halSensorIFParam[2].scenarioId;
        sensorPara[2].u1IsBypassSensorScenario = (MUINT8)halSensorIFParam[2].isBypassScenario;
        sensorPara[2].u1IsContinuous = (MUINT8)halSensorIFParam[2].isContinuous;
        sensorPara[2].u1MIPIDataTermDelay = sensorDrvInfo[2].MIPIDataLowPwr2HighSpeedTermDelayCount;
        //sensorPara[2].u1MIPIDataSettleDelay = sensorDrvInfo[2].MIPIDataLowPwr2HighSpeedSettleDelayCount;
        sensorPara[2].u1MIPIClkTermDelay = sensorDrvInfo[2].MIPICLKLowPwr2HighSpeedTermDelayCount;
        sensorPara[2].u1MIPILaneNum = sensorDrvInfo[2].SensorMIPILaneNumber;
        sensorPara[2].u1MIPIPacketECCOrder = sensorDrvInfo[2].SensorPacketECCOrder;
        sensorPara[2].MIPI_OPHY_TYPE = (MUINT)sensorDrvInfo[2].MIPIsensorType;
        sensorPara[2].NCSI2_HSRXDET_MODE = (MUINT)sensorDrvInfo[2].SettleDelayMode;
        sensorPara[2].cropWidth = halSensorIFParam[idx].crop.w;
        sensorPara[2].cropHeight = halSensorIFParam[idx].crop.h;
        sensorPara[2].frameRate = halSensorIFParam[idx].framerate;
        sensorPara[2].twopixelOn = halSensorIFParam[idx].twopixelOn;
        sensorPara[2].debugMode = halSensorIFParam[idx].debugMode;
        sensorPara[2].HDRMode = halSensorIFParam[idx].HDRMode;
        sensorPara[2].SCAM_DataNumber = (MUINT)sensorDrvInfo[2].SCAM_DataNumber;
        sensorPara[2].SCAM_DDR_En = (MUINT)sensorDrvInfo[2].SCAM_DDR_En;
        sensorPara[2].SCAM_CLK_INV = (MUINT)sensorDrvInfo[2].SCAM_CLK_INV;
    }


    //main  sensor
    if(currSensorDev & SENSOR_DEV_MAIN) {
        eSensorDev = SENSOR_MAIN;


        switch (sensorPara[0].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_PRV;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_PRV;
                srcWidth = sensorDrvInfo[0].SensorPreviewWidth;
                srcHeight = sensorDrvInfo[0].SensorPreviewHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CAP;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CAP;
                srcWidth = sensorDrvInfo[0].SensorCapWidth;
                srcHeight = sensorDrvInfo[0].SensorCapHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_VD;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_VD;
                srcWidth = sensorDrvInfo[0].SensorVideoWidth;
                srcHeight = sensorDrvInfo[0].SensorVideoHeight;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_VD1;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_VD1;
                srcWidth = sensorDrvInfo[0].SensorVideo1Width;
                srcHeight = sensorDrvInfo[0].SensorVideo1Height;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_VD2;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_VD2;
                srcWidth = sensorDrvInfo[0].SensorVideo2Width;
                srcHeight = sensorDrvInfo[0].SensorVideo2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CST1;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CST1;
                srcWidth = sensorDrvInfo[0].SensorCustom1Width;
                srcHeight = sensorDrvInfo[0].SensorCustom1Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CST2;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CST2;
                srcWidth = sensorDrvInfo[0].SensorCustom2Width;
                srcHeight = sensorDrvInfo[0].SensorCustom2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CST3;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CST3;
                srcWidth = sensorDrvInfo[0].SensorCustom3Width;
                srcHeight = sensorDrvInfo[0].SensorCustom3Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CST4;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CST4;
                srcWidth = sensorDrvInfo[0].SensorCustom4Width;
                srcHeight = sensorDrvInfo[0].SensorCustom4Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                pixelX0 = sensorDrvInfo[0].SensorGrabStartX_CST5;
                pixelY0 = sensorDrvInfo[0].SensorGrabStartY_CST5;
                srcWidth = sensorDrvInfo[0].SensorCustom5Width;
                srcHeight = sensorDrvInfo[0].SensorCustom5Height;
                break;
            default:
                MY_LOGE("main camera incorrect scenario");
                break;

        }


        ret = pSensorDrv->sendCommand(eSensorDev, CMD_SENSOR_GET_INPUT_BIT_ORDER, (MUINTPTR)&inDataFmt);

        // Source is from sensor
        if (sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_RAW) {
            // RAW
            sensorPara[0].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[0].crop.w)>>1);
            //ef test, use test model, tg in/imgo out
            if((efTestEnable == 1) || (efTestEnable == 2)  || (efTestEnable == 3))
            {
                sensorPara[0].u4PixelX0 = 4;//pixelX0 + ((srcWidth - halSensorIFParam[0].crop.w)>>1);
            }
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;
            sensorPara[0].padSel = PAD_10BIT;//pad2cam_data_sel
            sensorPara[0].inDataType = RAW_10BIT_FMT;//cam_tg_input_fmt
            sensorPara[0].senInLsb = TG_12BIT;//cam_tg_path_cfg
        }
        else if (sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_RAW8) {
            // RAW
            sensorPara[0].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[0].crop.w)>>1);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            if (inDataFmt == 0) {
                sensorPara[0].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[0].padSel = PAD_8BIT_7_0;
            }
            sensorPara[0].inDataType = RAW_8BIT_FMT;
            sensorPara[0].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_RAW12) {
            // RAW
            sensorPara[0].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[0].crop.w)>>1);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            sensorPara[0].padSel = PAD_10BIT;
            sensorPara[0].inDataType = RAW_12BIT_FMT;
            sensorPara[0].senInLsb = TG_12BIT;
        }
        else if ((sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_YUV)||(sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_YCBCR)){
            // Yuv422 or YCbCr
            sensorPara[0].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[0].crop.w);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w * 2;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            if (inDataFmt == 0) {
                sensorPara[0].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[0].padSel = PAD_8BIT_7_0;
            }
            sensorPara[0].inDataType = YUV422_FMT;
            sensorPara[0].senInLsb = TG_8BIT;
        }
        else if(sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_RGB565) {
            // RGB565
            sensorPara[0].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[0].crop.w);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w * 2;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            if (inDataFmt == 0) {
                sensorPara[0].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[0].padSel = PAD_8BIT_7_0;
            }
            sensorPara[0].inDataType = RGB565_MIPI_FMT;
            sensorPara[0].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_RGB888) {
            // RGB888
            sensorPara[0].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[0].crop.w);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w * 2;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            if (inDataFmt == 0) {
                sensorPara[0].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[0].padSel = PAD_8BIT_7_0;
            }
            sensorPara[0].inDataType = RGB888_MIPI_FMT;
            sensorPara[0].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[0].sensorType == IMAGE_SENSOR_TYPE_JPEG) {
            sensorPara[0].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[0].crop.w);
            sensorPara[0].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[0].crop.h)>>1);
            sensorPara[0].u4PixelX1 = sensorPara[0].u4PixelX0 + halSensorIFParam[0].crop.w * 2;
            sensorPara[0].u4PixelY1 = sensorPara[0].u4PixelY0 + halSensorIFParam[0].crop.h;

            if (inDataFmt == 0) {
                sensorPara[0].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[0].padSel = PAD_8BIT_7_0;
            }
            sensorPara[0].inDataType = JPEG_FMT;
            sensorPara[0].senInLsb = TG_8BIT;

        }
        else  {

        }

        if (sensorDrvInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
            sensorPara[0].inSrcTypeSel = MIPI_SENSOR;
        }
        else if (sensorDrvInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
            sensorPara[0].inSrcTypeSel = PARALLEL_SENSOR;
        }
        else if (sensorDrvInfo[0].SensroInterfaceType == SENSOR_INTERFACE_TYPE_SERIAL) {
            sensorPara[0].inSrcTypeSel = SERIAL_SENSOR;
        }
        else {
            sensorPara[0].inSrcTypeSel = TEST_MODEL;
        }

        if(sensorPara[0].debugMode == 1)
        {
            sensorPara[0].inSrcTypeSel  = TEST_MODEL;
        }
        //VR
        if(sensorPara[0].HDRMode == 2)
        {
            sensorPara[0].inSrcTypeSel  = VIRTUAL_CHANNEL_1;
        }

        sensorPara[0].u1HsyncPol = sensorDrvInfo[0].SensorHsyncPolarity;
        sensorPara[0].u1VsyncPol = sensorDrvInfo[0].SensorVsyncPolarity;




    }


    // SUB sensor
    if(currSensorDev & SENSOR_DEV_SUB ) {
        eSensorDev = SENSOR_SUB;


        switch (sensorPara[1].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_PRV;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_PRV;
                srcWidth = sensorDrvInfo[1].SensorPreviewWidth;
                srcHeight = sensorDrvInfo[1].SensorPreviewHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CAP;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CAP;
                srcWidth = sensorDrvInfo[1].SensorCapWidth;
                srcHeight = sensorDrvInfo[1].SensorCapHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_VD;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_VD;
                srcWidth = sensorDrvInfo[1].SensorVideoWidth;
                srcHeight = sensorDrvInfo[1].SensorVideoHeight;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_VD1;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_VD1;
                srcWidth = sensorDrvInfo[1].SensorVideo1Width;
                srcHeight = sensorDrvInfo[1].SensorVideo1Height;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_VD2;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_VD2;
                srcWidth = sensorDrvInfo[1].SensorVideo2Width;
                srcHeight = sensorDrvInfo[1].SensorVideo2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CST1;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CST1;
                srcWidth = sensorDrvInfo[1].SensorCustom1Width;
                srcHeight = sensorDrvInfo[1].SensorCustom1Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CST2;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CST2;
                srcWidth = sensorDrvInfo[1].SensorCustom2Width;
                srcHeight = sensorDrvInfo[1].SensorCustom2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CST3;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CST3;
                srcWidth = sensorDrvInfo[1].SensorCustom3Width;
                srcHeight = sensorDrvInfo[1].SensorCustom3Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CST4;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CST4;
                srcWidth = sensorDrvInfo[1].SensorCustom4Width;
                srcHeight = sensorDrvInfo[1].SensorCustom4Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                pixelX0 = sensorDrvInfo[1].SensorGrabStartX_CST5;
                pixelY0 = sensorDrvInfo[1].SensorGrabStartY_CST5;
                srcWidth = sensorDrvInfo[1].SensorCustom5Width;
                srcHeight = sensorDrvInfo[1].SensorCustom5Height;
                break;
            default:
                MY_LOGE("sub camera incorrect scenario");
                break;
        }

        ret = pSensorDrv->sendCommand(eSensorDev, CMD_SENSOR_GET_INPUT_BIT_ORDER, (MUINTPTR)&inDataFmt);

        // Source is from sensor
        if (sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_RAW) {
            // RAW
            sensorPara[1].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[1].crop.w)>>1);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;

            sensorPara[1].padSel = PAD_10BIT;
            sensorPara[1].inDataType = RAW_10BIT_FMT;
            sensorPara[1].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_RAW8) {
            // RAW
            sensorPara[1].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[1].crop.w)>>1);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;

            if (inDataFmt == 0) {
                sensorPara[1].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[1].padSel = PAD_8BIT_7_0;
            }
            sensorPara[1].inDataType = RAW_8BIT_FMT;
            sensorPara[1].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_RAW12) {
            // RAW
            sensorPara[1].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[1].crop.w)>>1);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;

            sensorPara[1].padSel = PAD_10BIT;
            sensorPara[1].inDataType = RAW_12BIT_FMT;
            sensorPara[1].senInLsb = TG_12BIT;
        }
        else if ((sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_YUV)||(sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_YCBCR)){
            // Yuv422 or YCbCr
            sensorPara[1].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[1].crop.w);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w * 2;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;

            if (inDataFmt == 0) {
                sensorPara[1].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[1].padSel = PAD_8BIT_7_0;
            }
            sensorPara[1].inDataType = YUV422_FMT;
            sensorPara[1].senInLsb = TG_8BIT;
        }
        else if(sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_RGB565) {
            // RGB565
            sensorPara[1].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[1].crop.w);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w * 2;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;

            if (inDataFmt == 0) {
                sensorPara[1].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[1].padSel = PAD_8BIT_7_0;
            }
            sensorPara[1].inDataType = RGB565_MIPI_FMT;
            sensorPara[1].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_RGB888) {
            // RGB888
            sensorPara[1].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[1].crop.w);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w * 2;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;


            if (inDataFmt == 0) {
                sensorPara[1].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[1].padSel = PAD_8BIT_7_0;
            }
            sensorPara[1].inDataType = RGB888_MIPI_FMT;
            sensorPara[1].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[1].sensorType == IMAGE_SENSOR_TYPE_JPEG) {
            sensorPara[1].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[1].crop.w);
            sensorPara[1].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[1].crop.h)>>1);
            sensorPara[1].u4PixelX1 = sensorPara[1].u4PixelX0 + halSensorIFParam[1].crop.w * 2;
            sensorPara[1].u4PixelY1 = sensorPara[1].u4PixelY0 + halSensorIFParam[1].crop.h;


            if (inDataFmt == 0) {
                sensorPara[1].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[1].padSel = PAD_8BIT_7_0;
            }
            sensorPara[1].inDataType = JPEG_FMT;
            sensorPara[1].senInLsb = TG_8BIT;

        }
        else  {

        }

        if (sensorDrvInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
            sensorPara[1].inSrcTypeSel = MIPI_SENSOR;
        }
        else if (sensorDrvInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
            sensorPara[1].inSrcTypeSel = PARALLEL_SENSOR;
        }
        else if (sensorDrvInfo[1].SensroInterfaceType == SENSOR_INTERFACE_TYPE_SERIAL) {
            sensorPara[1].inSrcTypeSel = SERIAL_SENSOR;
        }
        else {
            sensorPara[1].inSrcTypeSel = TEST_MODEL;
        }

        if(sensorPara[1].debugMode == 1)
        {
            sensorPara[1].inSrcTypeSel  = TEST_MODEL;
        }
        sensorPara[1].u1HsyncPol = sensorDrvInfo[1].SensorHsyncPolarity;
        sensorPara[1].u1VsyncPol = sensorDrvInfo[1].SensorVsyncPolarity;



    }


    // MAIN_2 sensor
    if(currSensorDev & SENSOR_DEV_MAIN_2 ) {
        eSensorDev = SENSOR_MAIN_2;



        switch (sensorPara[2].scenarioId) {
            case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_PRV;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_PRV;
                srcWidth = sensorDrvInfo[2].SensorPreviewWidth;
                srcHeight = sensorDrvInfo[2].SensorPreviewHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CAP;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CAP;
                srcWidth = sensorDrvInfo[2].SensorCapWidth;
                srcHeight = sensorDrvInfo[2].SensorCapHeight;
                break;
            case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_VD;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_VD;
                srcWidth = sensorDrvInfo[2].SensorVideoWidth;
                srcHeight = sensorDrvInfo[2].SensorVideoHeight;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_VD1;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_VD1;
                srcWidth = sensorDrvInfo[2].SensorVideo1Width;
                srcHeight = sensorDrvInfo[2].SensorVideo1Height;
                break;
            case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_VD2;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_VD2;
                srcWidth = sensorDrvInfo[2].SensorVideo2Width;
                srcHeight = sensorDrvInfo[2].SensorVideo2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM1:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CST1;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CST1;
                srcWidth = sensorDrvInfo[2].SensorCustom1Width;
                srcHeight = sensorDrvInfo[2].SensorCustom1Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM2:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CST2;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CST2;
                srcWidth = sensorDrvInfo[2].SensorCustom2Width;
                srcHeight = sensorDrvInfo[2].SensorCustom2Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM3:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CST3;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CST3;
                srcWidth = sensorDrvInfo[2].SensorCustom3Width;
                srcHeight = sensorDrvInfo[2].SensorCustom3Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM4:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CST4;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CST4;
                srcWidth = sensorDrvInfo[2].SensorCustom4Width;
                srcHeight = sensorDrvInfo[2].SensorCustom4Height;
                break;
            case SENSOR_SCENARIO_ID_CUSTOM5:
                pixelX0 = sensorDrvInfo[2].SensorGrabStartX_CST5;
                pixelY0 = sensorDrvInfo[2].SensorGrabStartY_CST5;
                srcWidth = sensorDrvInfo[2].SensorCustom5Width;
                srcHeight = sensorDrvInfo[2].SensorCustom5Height;
                break;
            default:
                MY_LOGE("main2 camera incorrect scenario");
                break;


        }

        ret = pSensorDrv->sendCommand(eSensorDev, CMD_SENSOR_GET_INPUT_BIT_ORDER, (MUINTPTR)&inDataFmt);



        // Source is from sensor
        if (sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_RAW) {
            // RAW
            sensorPara[2].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[2].crop.w)>>1);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            sensorPara[2].padSel = PAD_10BIT;
            sensorPara[2].inDataType = RAW_10BIT_FMT;
            sensorPara[2].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_RAW8) {
            // RAW
            sensorPara[2].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[2].crop.w)>>1);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            if (inDataFmt == 0) {
                sensorPara[2].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[2].padSel = PAD_8BIT_7_0;
            }
            sensorPara[2].inDataType = RAW_8BIT_FMT;
            sensorPara[2].senInLsb = TG_12BIT;
        }
        else if (sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_RAW12) {
            // RAW
            sensorPara[2].u4PixelX0 = pixelX0 + ((srcWidth - halSensorIFParam[2].crop.w)>>1);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            sensorPara[2].padSel = PAD_10BIT;
            sensorPara[2].inDataType = RAW_12BIT_FMT;
            sensorPara[2].senInLsb = TG_12BIT;
        }
        else if ((sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_YUV)||(sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_YCBCR)){
            // Yuv422 or YCbCr
            sensorPara[2].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[2].crop.w);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w * 2;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            if (inDataFmt == 0) {
                sensorPara[2].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[2].padSel = PAD_8BIT_7_0;
            }
            sensorPara[2].inDataType = YUV422_FMT;
            sensorPara[2].senInLsb = TG_8BIT;
        }
        else if(sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_RGB565) {
            // RGB565
            sensorPara[2].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[2].crop.w);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w * 2;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            if (inDataFmt == 0) {
                sensorPara[2].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[2].padSel = PAD_8BIT_7_0;
            }
            sensorPara[2].inDataType = RGB565_MIPI_FMT;
            sensorPara[2].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_RGB888) {
            // RGB888
            sensorPara[2].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[2].crop.w);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w * 2;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            if (inDataFmt == 0) {
                sensorPara[2].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[2].padSel = PAD_8BIT_7_0;
            }
            sensorPara[2].inDataType = RGB888_MIPI_FMT;
            sensorPara[2].senInLsb = TG_8BIT;

        }
        else if(sensorRawInfo[2].sensorType == IMAGE_SENSOR_TYPE_JPEG) {
            sensorPara[2].u4PixelX0 = pixelX0 + (srcWidth - halSensorIFParam[2].crop.w);
            sensorPara[2].u4PixelY0 = pixelY0 + ((srcHeight - halSensorIFParam[2].crop.h)>>1);
            sensorPara[2].u4PixelX1 = sensorPara[2].u4PixelX0 + halSensorIFParam[2].crop.w * 2;
            sensorPara[2].u4PixelY1 = sensorPara[2].u4PixelY0 + halSensorIFParam[2].crop.h;

            if (inDataFmt == 0) {
                sensorPara[2].padSel = PAD_8BIT_9_2;
            }
            else {
                sensorPara[2].padSel = PAD_8BIT_7_0;
            }
            sensorPara[2].inDataType = JPEG_FMT;
            sensorPara[2].senInLsb = TG_8BIT;

        }
        else  {

        }

        if (sensorDrvInfo[2].SensroInterfaceType == SENSOR_INTERFACE_TYPE_MIPI) {
            sensorPara[2].inSrcTypeSel = MIPI_SENSOR;
        }
        else if (sensorDrvInfo[2].SensroInterfaceType == SENSOR_INTERFACE_TYPE_PARALLEL) {
            sensorPara[2].inSrcTypeSel = PARALLEL_SENSOR;
        }
        else if (sensorDrvInfo[2].SensroInterfaceType == SENSOR_INTERFACE_TYPE_SERIAL) {
            sensorPara[2].inSrcTypeSel = SERIAL_SENSOR;
        }
        else {
            sensorPara[2].inSrcTypeSel = TEST_MODEL;
        }

        if(sensorPara[2].debugMode == 1)
        {
            sensorPara[2].inSrcTypeSel  = TEST_MODEL;
        }

        sensorPara[2].u1HsyncPol = sensorDrvInfo[2].SensorHsyncPolarity;
        sensorPara[2].u1VsyncPol = sensorDrvInfo[2].SensorVsyncPolarity;


    }


    //Determine one pixel/two pixel
    pixelModeArrange(currSensorDev);

    ret = seninfControl(1, currSensorDev);

    if(currSensorDev & SENSOR_DEV_MAIN ) {
        eSensorDev = SENSOR_MAIN;
        sensorScenarioId = halSensorIFParam[0].scenarioId;
        // set each sensor scenario separately
        g_ScenarioconfMain.sId = sensorScenarioId;
        g_ScenarioconfMain.sensorDevId = eSensorDev;
        g_ScenarioconfMain.InitFPS = halSensorIFParam[0].framerate;
        g_ScenarioconfMain.HDRMode = halSensorIFParam[0].HDRMode;

        ret = pSensorDrv->setScenario(g_ScenarioconfMain);
        if (ret < 0) {
            MY_LOGE("camera(%d) halSensorSetScenario fail ",eSensorDev);
            goto lbExit;
        }
        //
      /*  if(hwsyncEnable == 1)
        {
            //hwsync drv, sendCommand that the sensor is configured
            ret=mpHwSyncDrv->sendCommand(HW_SYNC_CMD_SET_MODE,eSensorDev, sensorScenarioId,currFPS);
            if(ret<0)
            {
                MY_LOGE("camera(%d) mpHwSyncDrv sendCommand fail ",eSensorDev);
                goto lbExit;
            }
        }*/
    }


    if(currSensorDev & SENSOR_DEV_SUB ) {
        eSensorDev = SENSOR_SUB;
        sensorScenarioId = halSensorIFParam[1].scenarioId;

        //set each sensor scenario separately
        g_ScenarioconfSub.sId = sensorScenarioId;
        g_ScenarioconfSub.sensorDevId = eSensorDev;
        g_ScenarioconfSub.InitFPS = halSensorIFParam[1].framerate;
        g_ScenarioconfSub.HDRMode = halSensorIFParam[1].HDRMode;
        ret = pSensorDrv->setScenario(g_ScenarioconfSub);

        if (ret < 0) {
            MY_LOGE("camera(%d) halSensorSetScenario fail ",eSensorDev);
            goto lbExit;
        }
        //
      /*  if(hwsyncEnable == 1)
        {
            //hwsync drv, sendCommand that the sensor is configured
            ret=mpHwSyncDrv->sendCommand(HW_SYNC_CMD_SET_MODE,eSensorDev, sensorScenarioId,currFPS);
            if(ret<0)
            {
                MY_LOGE("camera(%d) mpHwSyncDrv sendCommand fail ",eSensorDev);
                goto lbExit;
            }
        }*/
    }

    if(currSensorDev & SENSOR_DEV_MAIN_2 ) {
        eSensorDev = SENSOR_MAIN_2;
        sensorScenarioId = halSensorIFParam[2].scenarioId;

        // set each sensor scenario separately
        g_ScenarioconfMain2.sId = sensorScenarioId;
        g_ScenarioconfMain2.sensorDevId = eSensorDev;
        g_ScenarioconfMain2.InitFPS = halSensorIFParam[2].framerate;
        g_ScenarioconfMain2.HDRMode = halSensorIFParam[2].HDRMode;
        ret = pSensorDrv->setScenario(g_ScenarioconfMain2);

        if (ret < 0) {
            MY_LOGE("camera(%d) halSensorSetScenario fail",eSensorDev);
            goto lbExit;
        }
        //
   /*     if(hwsyncEnable == 1)
        {
            //hwsync drv, sendCommand that the sensor is configured
            ret=mpHwSyncDrv->sendCommand(HW_SYNC_CMD_SET_MODE,eSensorDev, sensorScenarioId, currFPS);
            if(ret<0)
            {
                MY_LOGE("camera(%d) mpHwSyncDrv sendCommand fail ",eSensorDev);
                goto lbExit;
            }
        }*/
    }



lbExit:
#ifndef USING_MTK_LDVT
    //CPTLog(Event_Sensor_setScenario, CPTFlagEnd);
#endif


    return ret;
}


 MINT HalSensor::setTgPhase(MINT32 sensorIdx, MINT32 pcEn) //CMMCLK: Main/sub, CMMCLK2:Main_2 (external signal design is not sync with internal signal in TG/I2C)
 {
     MINT32 ret = 0, ret2 = 0;
     MUINT32 u4PadPclkInv1 = 0, u4PadPclkInv2 = 0, u4PadPclkInv3 = 0;
     MINT32 clkInKHz1=0, clkCnt1=0, mclk1=0, mclkSel1=0;
     MINT32 clkInKHz2=0, clkCnt2=0, mclk2=0, mclkSel2=0;
     MINT32 clkInKHz3=0, clkCnt3=0, mclk3=0, mclkSel3=0;

     SensorDrv *const pSensorDrv = SensorDrv::get();
     SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();

     MY_LOGD("[setTgPhase] Tg1clk: %d, Tg2clk: %d, Tg3clk : %d \n", sensorDrvInfo[0].SensorClockFreq, sensorDrvInfo[1].SensorClockFreq,sensorDrvInfo[2].SensorClockFreq);
     MY_LOGD("sensorDev = %d, pcEn = %d\n", sensorIdx, pcEn);//JH debug

     /*
             SENSOR_DEV_NONE = 0x00,
             SENSOR_DEV_MAIN = 0x01,
             SENSOR_DEV_SUB  = 0x02,
             SENSOR_DEV_PIP = 0x03,
             SENSOR_DEV_MAIN_2 = 0x04,
             SENSOR_DEV_MAIN_3D = 0x05,
     */
     if(sensorIdx & SENSOR_DEV_MAIN) {
         clkInKHz1 = sensorDrvInfo[0].SensorClockFreq * 1000;

         if ((clkInKHz1 < 3250) || (clkInKHz1 >= 104000)) {
             MY_LOGE("Err-Input clock rate error, %d \n", clkInKHz1);
             return -EINVAL;
         }
         //
         if ((48000 % clkInKHz1) == 0) {            // Clock is in 48MHz group, original source is 48MHz
             mclk1 = 48000;
             mclkSel1 = CAM_PLL_48_GROUP;
         }
         else {
             // Clock is in 52MHz group
             mclk1 = 208000;//52000;
             mclkSel1 = CAM_PLL_52_GROUP;
         }

         //
         clkCnt1 = (mclk1 + (clkInKHz1 >> 1)) / clkInKHz1;
         // Maximum CLKCNT is 15
         clkCnt1 = clkCnt1 > 15 ? 15 : clkCnt1-1;
         MY_LOGD("  mclk1: %d, clkCnt1: %d \n", mclk1, clkCnt1);
     }

     if(sensorIdx & SENSOR_DEV_SUB) {
         clkInKHz2 = sensorDrvInfo[1].SensorClockFreq * 1000;


         if ((clkInKHz2 < 3250) || (clkInKHz2 >= 104000)) {
             MY_LOGE("Err-Input clock rate error, %d \n", clkInKHz2);
             return -EINVAL;
         }
         //
         if ((48000 % clkInKHz2) == 0) {
             // Clock is in 48MHz group, original source is 48MHz
             mclk2 = 48000;
             mclkSel2 = CAM_PLL_48_GROUP;
         }
         else {
             // Clock is in 52MHz group
             mclk2 = 208000;//52000;
             mclkSel2 = CAM_PLL_52_GROUP;
         }

         //
         clkCnt2 = (mclk2 + (clkInKHz2 >> 1)) / clkInKHz2;
         clkCnt2 = clkCnt2 > 15 ? 15 : clkCnt2-1;
         MY_LOGD("  mclk1: %d, clkCnt1: %d \n", mclk2, clkCnt2);
     }


     if(sensorIdx & SENSOR_DEV_MAIN_2){

         clkInKHz3 = sensorDrvInfo[2].SensorClockFreq * 1000;
         if ((clkInKHz3 < 3250) || (clkInKHz3 >= 104000)) {
             MY_LOGE("Err-Input clock rate error, %d \n", clkInKHz3);
             return -EINVAL;
         }
         //
         if ((48000 % clkInKHz3) == 0) {
             // Clock is in 48MHz group, original source is 48MHz
             mclk3 = 48000;
             mclkSel3 = CAM_PLL_48_GROUP;
         }
         else {
             mclk3 = 208000;
             mclkSel3 = CAM_PLL_52_GROUP;
         }

         //
         clkCnt3 = (mclk3 + (clkInKHz3 >> 1)) / clkInKHz3;
         // Maximum CLKCNT is 15
         clkCnt3 = clkCnt3 > 15 ? 15 : clkCnt3-1;
         MY_LOGD("  mclk2: %d, clkCnt2: %d \n", mclk3, clkCnt3);
     }


     switch (sensorIdx)
     {
     case SENSOR_DEV_MAIN:
         ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv1);
         break;
     case SENSOR_DEV_SUB:
         ret = pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv2);
         break;
     case SENSOR_DEV_PIP:
         ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv1);
         ret2 = pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv2);
         break;
     case SENSOR_DEV_MAIN_2:
         ret = pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv3);
         break;
     case SENSOR_DEV_MAIN_3D:
         ret = pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv1);
         ret2 = pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_PAD_PCLK_INV, (MUINTPTR)&u4PadPclkInv3);
         break;
     default:
         u4PadPclkInv1 = 0;
         u4PadPclkInv2 = 0;
         u4PadPclkInv3 = 0;
         ret = 0;
         ret2 = 0;
         break;
     }
     if ((ret < 0)||(ret2 < 0)) {
         MY_LOGE("CMD_SENSOR_GET_PAD_PCLK_INV fail - err(%x), err2(%x)\n", ret, ret2);
     }
     MY_LOGD("[setTgPhase] u4PadPclkInv_1(%d),u4PadPclkInv_2(%d) \n", u4PadPclkInv1,u4PadPclkInv2);

     // Config TG, always use Camera PLL, 1: 48MHz, 2: 104MHz

     if(sensorIdx & SENSOR_DEV_MAIN ) {
         if(sensorPara[0].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1(
                 pcEn, mclkSel1 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt1, sensorDrvInfo[0].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[0].SensorClockFallingCount, sensorDrvInfo[0].SensorClockRisingCount, u4PadPclkInv1);
         }
         else if(sensorPara[0].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2(
                 pcEn, mclkSel1 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt1, sensorDrvInfo[0].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[0].SensorClockFallingCount, sensorDrvInfo[0].SensorClockRisingCount, u4PadPclkInv1);
         }
         else if(sensorPara[0].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3(
                 pcEn, mclkSel1 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt1, sensorDrvInfo[0].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[0].SensorClockFallingCount, sensorDrvInfo[0].SensorClockRisingCount, u4PadPclkInv1);
         }
         else {
             MY_LOGE("Main camera mclk source (%d) incorrrect. Please check cfg_setting_imgsensor.cpp\n",sensorPara[0].mclkSrc );
         }

         if (ret < 0) {
             MY_LOGE("setTg1PhaseCounter fail\n");
             return ret;
         }
     }


     //notice SUB sensorInfo[1] but use Tg1 mclk
     if(sensorIdx & SENSOR_DEV_SUB) {
         if(sensorPara[1].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3(
                 pcEn, mclkSel2 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt2, sensorDrvInfo[1].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[1].SensorClockFallingCount, sensorDrvInfo[1].SensorClockRisingCount, u4PadPclkInv2);
         }
         else if(sensorPara[1].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1(
                 pcEn, mclkSel2 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt2, sensorDrvInfo[1].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[1].SensorClockFallingCount, sensorDrvInfo[1].SensorClockRisingCount, u4PadPclkInv2);
         }
         else if(sensorPara[1].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2(
                 pcEn, mclkSel2 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt2, sensorDrvInfo[1].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[1].SensorClockFallingCount, sensorDrvInfo[1].SensorClockRisingCount, u4PadPclkInv2);
         }
         else {
             MY_LOGE("Sub camera mclk source (%d) incorrrect. Please check cfg_setting_imgsensor.cpp\n",sensorPara[1].mclkSrc );
         }

         if (ret < 0) {
             MY_LOGE("setTg3PhaseCounter fail\n");
             return ret;
         }

     }



     if(sensorIdx & SENSOR_DEV_MAIN_2){
         if(sensorPara[2].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2(
                 pcEn, mclkSel3 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt3, sensorDrvInfo[2].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[2].SensorClockFallingCount, sensorDrvInfo[2].SensorClockRisingCount, u4PadPclkInv3);
         }
         else if(sensorPara[2].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1(
                 pcEn, mclkSel3 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt3, sensorDrvInfo[2].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[2].SensorClockFallingCount, sensorDrvInfo[2].SensorClockRisingCount, u4PadPclkInv3);
         }
         if(sensorPara[2].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3(
                 pcEn, mclkSel3 /*sensorInfo.SensorMasterClockSwitch ? 0 : 1*/,
                 clkCnt3, sensorDrvInfo[2].SensorClockPolarity ? 0 : 1,
                 sensorDrvInfo[2].SensorClockFallingCount, sensorDrvInfo[2].SensorClockRisingCount, u4PadPclkInv3);
         }
         else {
             MY_LOGE("Main2 camera mclk source (%d) incorrrect. Please check cfg_setting_imgsensor.cpp\n",sensorPara[2].mclkSrc );
         }

         if (ret < 0) {
             MY_LOGE("setTg2PhaseCounter fail\n");
             return ret;
         }
     }



     pSeninfDrv->destroyInstance();

     return ret;
 }





MINT HalSensor::sendCommand(
    MUINT sensorDevIdx,
    MUINTPTR cmd,
    MUINTPTR arg1,
    MUINTPTR arg2,
    MUINTPTR arg3)
{
    MINT32 ret = 0;
    MUINT32 cmdId = 0;

    MUINT32 sensorDevId = sensorDevIdx;

    SensorDrv *const pSensorDrv = SensorDrv::get();
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
    switch (cmd) {
    //0x1000

    case SENSOR_CMD_SET_SENSOR_EXP_TIME:
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_TIME;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, (MUINTPTR)arg1);
        MY_LOGD("Exposure Time: %d \n", *(MUINT32 *) arg1);

        break;

    case SENSOR_CMD_SET_SENSOR_EXP_LINE:
        cmdId = CMD_SENSOR_SET_SENSOR_EXP_LINE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_FLICKER_FRAME_RATE:
        cmdId = CMD_SENSOR_SET_FLICKER_FRAME_RATE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_VIDEO_FRAME_RATE:
        cmdId = CMD_SENSOR_SET_VIDEO_FRAME_RATE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_AE_EXPOSURE_GAIN_SYNC:
        cmdId = CMD_SENSOR_SET_AE_EXPOSURE_GAIN_SYNC;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_CCT_FEATURE_CONTROL:
        cmdId = CMD_SENSOR_SET_CCT_FEATURE_CONTROL;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;

    case SENSOR_CMD_SET_SENSOR_CALIBRATION_DATA:
        cmdId = CMD_SENSOR_SET_SENSOR_CALIBRATION_DATA;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO:
        cmdId = CMD_SENSOR_SET_MAX_FRAME_RATE_BY_SCENARIO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        MY_LOGD("  SENSOR_CMD_SET_MAX_FRAME_RATE_BY_SCENARIO: scenario = %d, frame rates = %d (10base) \n", *(MUINT32 *) arg1,*(MUINT32 *) arg2);
        break;
    case SENSOR_CMD_SET_TEST_PATTERN_OUTPUT:
        cmdId = CMD_SENSOR_SET_TEST_PATTERN_OUTPUT;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_SENSOR_ESHUTTER_GAIN:
        cmdId = CMD_SENSOR_SET_ESHUTTER_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_OB_LOCK:
        cmdId = CMD_SENSOR_SET_OB_LOCK;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD:
        MY_LOGD("SENSOR_CMD_SET_SENSOR_OTP_AWB_CMD: %d", *(MUINT32 *) arg1);
        cmdId = CMD_SNESOR_SET_SENSOR_OTP_AWB_CMD;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_SENSOR_OTP_LSC_CMD:
        MY_LOGD("SENSOR_CMD_SET_SENSOR_OTP_LSC_CMD: %d", *(MUINT32 *) arg1);
        cmdId = CMD_SNESOR_SET_SENSOR_OTP_LSC_CMD;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_MIN_MAX_FPS:
        cmdId = CMD_SENSOR_SET_MIN_MAX_FPS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_SET_N3D_CONFIG:
        ret = pSeninfDrv->setN3DCfg(*(MUINT32 *) arg1,*((MUINT32 *)arg1+1),*((MUINT32 *)arg1+2),*((MUINT32 *)arg1+3),*((MUINT32 *)arg1+4),*((MUINT32 *)arg1+5));
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_CONFIG fail! \n");
        }
        break;
    case SENSOR_CMD_SET_N3D_I2C_POS:
        ret = pSeninfDrv->setN3DI2CPos(*(MUINTPTR*)arg1);
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_I2C_POS fail! \n");
        }
        break;
    case SENSOR_CMD_SET_N3D_I2C_TRIGGER:
        ret = pSeninfDrv->setN3DTrigger(*(MUINTPTR *)arg1, *(MUINTPTR *)arg2);
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_I2C_TRIGGER fail! \n");
        }
        break;
    case SENSOR_CMD_SET_SENSOR_HDR_SHUTTER_GAIN:
        cmdId = CMD_SENSOR_SET_IHDR_SHUTTER_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    case SENSOR_CMD_SET_SENSOR_HDR_SHUTTER:
        cmdId = CMD_SENSOR_SET_HDR_SHUTTER;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_SET_SENSOR_HDR_AWB_GAIN:
        cmdId = CMD_SENSOR_SET_SENSOR_AWB_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    //0x2000
    case SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT:
        cmdId = CMD_SENSOR_GET_UNSTABLE_DELAY_FRAME_CNT;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2); //arg1 = mode
        break;


    case SENSOR_CMD_GET_PIXEL_CLOCK_FREQ:
        cmdId = CMD_SENSOR_GET_PIXEL_CLOCK_FREQ;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_FRAME_SYNC_PIXEL_LINE_NUM:
        cmdId = CMD_SENSOR_GET_FRAME_SYNC_PIXEL_LINE_NUM;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_SENSOR_FEATURE_INFO:
        cmdId = CMD_SENSOR_GET_SENSOR_FEATURE_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;


    case SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO:
        cmdId = CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;


    case SENSOR_CMD_GET_TEST_PATTERN_CHECKSUM_VALUE:
        cmdId = CMD_SENSOR_GET_TEST_PATTERN_CHECKSUM_VALUE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_TEMPERATURE_VALUE:
        cmdId = CMD_SENSOR_GET_TEMPERATURE_VALUE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO:
        cmdId = CMD_SENSOR_GET_SENSOR_CROP_WIN_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        MY_LOGD("  SENSOR_CMD_GET_SENSOR_CROP_WIN_INFO: scenario = %d, crop = 0x%x  \n", *(MUINT32 *) arg1,*(MUINT32 *) arg2);
        break;
    case SENSOR_CMD_GET_SENSOR_PIXELMODE:
        *(MUINTPTR *)arg3  = GetpixelMode((MUINT)sensorDevId, *(MUINTPTR*)arg1, *(MUINTPTR *)arg2);
        MY_LOGD("SENSOR_CMD_GET_SENSOR_PIXELMODE:scenario = %d, fps = %d, Pixelmode =%d \n", *(MUINT32 *)arg1, *(MUINT32 *)arg2, *(MUINT32 *)arg3);
        break;

    case SENSOR_CMD_GET_SENSOR_N3D_DIFFERENCE_COUNT:
        ret = pSeninfDrv->getN3DDiffCnt((MUINT32 *)arg1);
        if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_SET_N3D_CONFIG fail! \n");
        }
        break;
   case SENSOR_CMD_GET_SENSOR_VC_INFO:
       ret = pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId, CMD_SENSOR_GET_SENSOR_VC_INFO, arg1, arg2);
       if(ret < 0) {
            MY_LOGE("[sendCommand] SENSOR_CMD_GET_SENSOR_VC_INFO fail! \n");
        }
       break;
   case SENSOR_CMD_GET_SENSOR_PDAF_INFO:
        cmdId = CMD_SENSOR_GET_SENSOR_PDAF_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        MY_LOGD("  SENSOR_CMD_GET_SENSOR_PDAF_INFO: scenario = %x %x\n", *(MUINT32 *) arg1,arg2);
        if(ret < 0) {
             MY_LOGE("[sendCommand] SENSOR_CMD_GET_SENSOR_PDAF_INFO fail! \n");
        }
        break;
    case SENSOR_CMD_GET_PDAF_DATA:
        cmdId = CMD_SENSOR_GET_PDAF_DATA;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    case SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY:
        cmdId = CMD_SENSOR_GET_SENSOR_PDAF_CAPACITY;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2, arg3);
        break;
    //0x3000
    case SENSOR_CMD_SET_YUV_FEATURE_CMD:
        cmdId = CMD_SENSOR_SET_YUV_FEATURE_CMD;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;

    case SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE:
        cmdId = CMD_SENSOR_SET_YUV_SINGLE_FOCUS_MODE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);
        break;

    case SENSOR_CMD_SET_YUV_CANCEL_AF:
        cmdId = CMD_SENSOR_SET_YUV_CANCEL_AF;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);
        break;

    case SENSOR_CMD_SET_YUV_CONSTANT_AF:
        cmdId = CMD_SENSOR_SET_YUV_CONSTANT_AF;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);
        break;

    case SENSOR_CMD_SET_YUV_INFINITY_AF:
            cmdId = CMD_SENSOR_SET_YUV_INFINITY_AF;
            pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId);
            break;

    case SENSOR_CMD_SET_YUV_AF_WINDOW:
        cmdId = CMD_SENSOR_SET_YUV_AF_WINDOW;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_YUV_AE_WINDOW:
        cmdId = CMD_SENSOR_SET_YUV_AE_WINDOW;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_SET_YUV_GAIN_AND_EXP_LINE:
        cmdId = CMD_SENSOR_SET_YUV_GAIN_AND_EXP_LINE;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;

    case SENSOR_CMD_SET_YUV_3A_CMD:
        cmdId = CMD_SENSOR_SET_YUV_3A_CMD;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    //0x4000
    case SENSOR_CMD_GET_YUV_AF_STATUS:
        cmdId = CMD_SENSOR_GET_YUV_AF_STATUS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AE_STATUS:
        cmdId = CMD_SENSOR_GET_YUV_AE_STATUS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AWB_STATUS:
        cmdId = CMD_SENSOR_GET_YUV_AWB_STATUS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_EV_INFO_AWB_REF_GAIN:
        cmdId = CMD_SENSOR_GET_YUV_EV_INFO_AWB_REF_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN:
        cmdId = CMD_SENSOR_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS:
        cmdId = CMD_SENSOR_GET_YUV_AF_MAX_NUM_FOCUS_AREAS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS:
        cmdId = CMD_SENSOR_GET_YUV_AE_MAX_NUM_METERING_AREAS;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_EXIF_INFO:
        cmdId = CMD_SENSOR_GET_YUV_EXIF_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;

    case SENSOR_CMD_GET_YUV_DELAY_INFO:
        cmdId = CMD_SENSOR_GET_YUV_DELAY_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_YUV_AE_AWB_LOCK:
        cmdId = CMD_SENSOR_GET_YUV_AE_AWB_LOCK_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_GET_YUV_STROBE_INFO:
        cmdId = CMD_SENSOR_GET_YUV_AE_FLASHLIGHT_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_YUV_TRIGGER_FLASHLIGHT_INFO:
        cmdId = CMD_SENSOR_GET_YUV_TRIGGER_FLASHLIGHT_INFO;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_YUV_AUTOTEST:
        cmdId = CMD_SENSOR_SET_YUV_AUTOTEST;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;
    case SENSOR_CMD_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME:
        cmdId=CMD_SENSOR_GET_SENSOR_N3D_STREAM_TO_VSYNC_TIME;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_N3D_I2C_STREAM_REGDATA:
        cmdId=CMD_SENSOR_SET_N3D_I2C_STREAM_REGDATA;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_N3D_START_STREAMING:
        cmdId = CMD_SENSOR_SET_N3D_START_STREAMING;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_SET_N3D_STOP_STREAMING:
        cmdId = CMD_SENSOR_SET_N3D_STOP_STREAMING;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1);
        break;
    case SENSOR_CMD_GET_SENSOR_ROLLING_SHUTTER:
        cmdId = CMD_SENSOR_GET_SENSOR_ROLLING_SHUTTER;
        pSensorDrv->sendCommand((SENSOR_DEV_ENUM)sensorDevId,cmdId, arg1, arg2);
        break;
    default:
        ret = -1;
        MY_LOGD("[sendCommand] err: 0x%x \n", cmd);
        break;
    }
    //

    pSeninfDrv->destroyInstance();


    return ret;

}


 MINT HalSensor::setSensorIODrivingCurrent(MINT32 sensorIdx)
 {

     MINT32 ret = 0;
     MINT32 increaseDivingCurrent1 = 0x08,increaseDivingCurrent2 = 0x08; // set to default 2mA and slew raw control

     SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();

     if( sensorIdx & SENSOR_DEV_MAIN ) { //Main/sub use TG1 mclk
         switch(sensorDrvInfo[0].SensorDrivingCurrent) {
             case ISP_DRIVING_2MA://4 //4mA
                 increaseDivingCurrent1 = 0x0;
                 break;
             case ISP_DRIVING_4MA:// 8mA
                 increaseDivingCurrent1 = 0x1;
                 break;
             case ISP_DRIVING_6MA://12mA
                 increaseDivingCurrent1 = 0x2;
                 break;
             case ISP_DRIVING_8MA://16mA
                 increaseDivingCurrent1 = 0x3;
                 break;
             default:
                 MY_LOGD("The driving current value is wrong\n");
                 break;
         }
         if(sensorPara[0].mclkSrc == 0) {
         ret = pSeninfDrv->setMclk1IODrivingCurrent(increaseDivingCurrent1);
     }
         else if(sensorPara[0].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2IODrivingCurrent(increaseDivingCurrent1);
         }
         else if(sensorPara[0].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3IODrivingCurrent(increaseDivingCurrent1);
         }
     }


     if( sensorIdx & SENSOR_DEV_SUB ) { //Main/sub use TG1 mclk
         switch(sensorDrvInfo[1].SensorDrivingCurrent) {
             case ISP_DRIVING_2MA://4 //4mA
                 increaseDivingCurrent1 = 0x0;
                 break;
             case ISP_DRIVING_4MA:// 8mA
                 increaseDivingCurrent1 = 0x1;
                 break;
             case ISP_DRIVING_6MA://12mA
                 increaseDivingCurrent1 = 0x2;
                 break;
             case ISP_DRIVING_8MA://16mA
                 increaseDivingCurrent1 = 0x3;
                 break;
             default:
                 MY_LOGD("The driving current value is wrong\n");
                 break;
         }
         if(sensorPara[1].mclkSrc == 2) {
         ret = pSeninfDrv->setMclk3IODrivingCurrent(increaseDivingCurrent1);
     }
         else if(sensorPara[1].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2IODrivingCurrent(increaseDivingCurrent1);
         }
         else if(sensorPara[1].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1IODrivingCurrent(increaseDivingCurrent1);
         }
     }
     if (ret < 0) {
         MY_LOGE("The Tg1 driving current setting is wrong\n");
     }


     if(sensorIdx & SENSOR_DEV_MAIN_2) {
         switch(sensorDrvInfo[2].SensorDrivingCurrent) {
             case ISP_DRIVING_2MA:
                 increaseDivingCurrent2 = 0x0;
                 break;
             case ISP_DRIVING_4MA:
                 increaseDivingCurrent2 = 0x1;
                 break;
             case ISP_DRIVING_6MA:
                 increaseDivingCurrent2 = 0x2;
                 break;
             case ISP_DRIVING_8MA:
                 increaseDivingCurrent2 = 0x3;
                 break;
             default:
                 MY_LOGD("The driving current value is wrong\n");
                 break;
         }

         if(sensorPara[2].mclkSrc == 1) {
             ret = pSeninfDrv->setMclk2IODrivingCurrent(increaseDivingCurrent1);
         }
         else if(sensorPara[2].mclkSrc == 2) {
             ret = pSeninfDrv->setMclk3IODrivingCurrent(increaseDivingCurrent1);
         }
         else if(sensorPara[2].mclkSrc == 0) {
             ret = pSeninfDrv->setMclk1IODrivingCurrent(increaseDivingCurrent1);
         }

     }

     if (ret < 0) {
         MY_LOGE("The Tg2 driving current setting is wrong\n");
     }

     pSeninfDrv->destroyInstance();

     return ret;
 }

MVOID HalSensor::pixelModeArrange(MUINT currSensorDev)
{
    //One pixel, two pixel
    if(currSensorDev & SENSOR_DEV_MAIN ) {
        if (sensorStaticInfo[0].sensorType == SENSOR_TYPE_RAW){
            if(((sensorPara[0].cropWidth * sensorPara[0].cropHeight/1000 * (sensorPara[0].frameRate/10)) > (TWO_PIXEL_MODE_THRESHOLD/10)) ||
                (sensorPara[0].twopixelOn)){
                sensorDynamicInfo[0].pixelMode = TWO_PIXEL_MODE;
            }
            else {
                sensorDynamicInfo[0].pixelMode = ONE_PIXEL_MODE;
            }
        }
        else if (sensorStaticInfo[0].sensorType == SENSOR_TYPE_JPEG) {
            sensorDynamicInfo[0].pixelMode = TWO_PIXEL_MODE;
        }
        else {
            sensorDynamicInfo[0].pixelMode = ONE_PIXEL_MODE;
        }
    }
   if(currSensorDev & SENSOR_DEV_SUB ) {
        if (sensorStaticInfo[1].sensorType == SENSOR_TYPE_RAW){
            if(((sensorPara[1].cropWidth * sensorPara[1].cropHeight/1000 * (sensorPara[1].frameRate/10)) > (TWO_PIXEL_MODE_THRESHOLD/10)) ||
                (sensorPara[1].twopixelOn)){
                sensorDynamicInfo[1].pixelMode = TWO_PIXEL_MODE;
            }
            else {
                sensorDynamicInfo[1].pixelMode = ONE_PIXEL_MODE;
            }
        }
        else if (sensorStaticInfo[1].sensorType == SENSOR_TYPE_JPEG) {
            sensorDynamicInfo[1].pixelMode = TWO_PIXEL_MODE;
        }
        else {
            sensorDynamicInfo[1].pixelMode = ONE_PIXEL_MODE;
        }
    }
   if(currSensorDev & SENSOR_DEV_MAIN_2 ) {
        if (sensorStaticInfo[2].sensorType == SENSOR_TYPE_RAW){
            if(((sensorPara[2].cropWidth * sensorPara[2].cropHeight/1000 * (sensorPara[2].frameRate/10)) > (TWO_PIXEL_MODE_THRESHOLD/10)) ||
                (sensorPara[2].twopixelOn)){
                sensorDynamicInfo[2].pixelMode = TWO_PIXEL_MODE;
            }
            else {
                sensorDynamicInfo[2].pixelMode = ONE_PIXEL_MODE;
            }
        }
        else if (sensorStaticInfo[2].sensorType == SENSOR_TYPE_JPEG) {
            sensorDynamicInfo[2].pixelMode = TWO_PIXEL_MODE;
        }
        else {
            sensorDynamicInfo[2].pixelMode = ONE_PIXEL_MODE;
        }
    }

       /////////////////////////////////////////////////////
    //get property for ef test
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.ef.test", value, "0");
    int efTestEnable=atoi(value);
    //ef test, use test model, tg in/imgo out
    if((efTestEnable == 1) || (efTestEnable == 2) || (efTestEnable == 3) )
    {
        sensorDynamicInfo[0].pixelMode = ONE_PIXEL_MODE;
    }
    /////////////////////////////////////////////////////


}

MINT HalSensor::GetpixelMode(MUINT currSensorDev, MUINT scenario, MUINT fps)
{
    MINT ret = 0;
    MUINT16 index;
    MUINT16 srcWidth=0;
    MUINT16 srcHeight=0;
    MUINT16 u16fps;

    if(currSensorDev & SENSOR_DEV_MAIN)
    {
        index = 0;
    }
    else if(currSensorDev & SENSOR_DEV_SUB)
    {
        index = 1;
    }
    else
    {
        index = 2;
    }

    switch (scenario) {
        case SENSOR_SCENARIO_ID_NORMAL_PREVIEW:
            srcWidth = sensorDrvInfo[index].SensorPreviewWidth;
            srcHeight = sensorDrvInfo[index].SensorPreviewHeight;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_CAPTURE:
            srcWidth = sensorDrvInfo[index].SensorCapWidth;
            srcHeight = sensorDrvInfo[index].SensorCapHeight;
            break;
        case SENSOR_SCENARIO_ID_NORMAL_VIDEO:
            srcWidth = sensorDrvInfo[index].SensorVideoWidth;
            srcHeight = sensorDrvInfo[index].SensorVideoHeight;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO1:
            srcWidth = sensorDrvInfo[index].SensorVideo1Width;
            srcHeight = sensorDrvInfo[index].SensorVideo1Height;
            break;
        case SENSOR_SCENARIO_ID_SLIM_VIDEO2:
            srcWidth = sensorDrvInfo[index].SensorVideo2Width;
            srcHeight = sensorDrvInfo[index].SensorVideo2Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM1:
            srcWidth = sensorDrvInfo[index].SensorCustom1Width;
            srcHeight = sensorDrvInfo[index].SensorCustom1Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM2:
            srcWidth = sensorDrvInfo[index].SensorCustom2Width;
            srcHeight = sensorDrvInfo[index].SensorCustom2Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM3:
            srcWidth = sensorDrvInfo[index].SensorCustom3Width;
            srcHeight = sensorDrvInfo[index].SensorCustom3Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM4:
            srcWidth = sensorDrvInfo[index].SensorCustom4Width;
            srcHeight = sensorDrvInfo[index].SensorCustom4Height;
            break;
        case SENSOR_SCENARIO_ID_CUSTOM5:
            srcWidth = sensorDrvInfo[index].SensorCustom5Width;
            srcHeight = sensorDrvInfo[index].SensorCustom5Height;
            break;
        default:
            MY_LOGE("camera incorrect scenario");
            break;
    }

    // Frame rate , Unit : FPS , Driver Unit : 10*FPS
    u16fps = 10*fps;
    //One pixel, two pixel
    if((srcWidth * srcHeight/1000 * (u16fps/10)) > (TWO_PIXEL_MODE_THRESHOLD/10))
    {
        ret = TWO_PIXEL_MODE;
    }
    else
    {
        ret = ONE_PIXEL_MODE;
    }

    /////////////////////////////////////////////////////
    //get property for ef test
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.ef.test", value, "0");
    int efTestEnable=atoi(value);
    //ef test, use test model, tg in/imgo out
    if((efTestEnable == 1) || (efTestEnable == 2) || (efTestEnable == 3) )
    {
        sensorDynamicInfo[0].pixelMode = ONE_PIXEL_MODE;
    }
    /////////////////////////////////////////////////////

    return ret;
}

MINT HalSensor::seninfControl(MUINT8 enable, MUINT32 currSensorDev)
{
    MINT ret = 0;
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
    MY_LOGD("seninfControl meSensorDev = %d, currSensorDev = %d, enable = %d, PixelMode=%d\n",meSensorDev,currSensorDev,enable,sensorDynamicInfo[0].pixelMode);
    MY_LOGD("sensorPara[0].inSrcTypeSel = %d, sensorPara[1].inSrcTypeSel=%d\n",sensorPara[0].inSrcTypeSel,sensorPara[1].inSrcTypeSel);

    if (1 == enable) {

        switch(meSensorDev) {
            case SENSOR_DEV_MAIN:
                if(meSensorDev != currSensorDev) {
                    MY_LOGE("seninfControl current control not match. meSensorDev = %d, currSensorDev = %d\n",meSensorDev,currSensorDev);
                }

                ret = pSeninfDrv->setSeninf1NCSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);    // Disable main's NCSI first
                ret = pSeninfDrv->setSeninf2NCSI2(0, 0, 0, 0, 0, 0, 0, 0, 0,0);   // Disable sub's NCSI first
                ret = pSeninfDrv->setTg1ViewFinderMode(0); //Disable tg1
                ret = pSeninfDrv->setTg2ViewFinderMode(0); //Disable tg2
                ret = pSeninfDrv->setTg1GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                        sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                if (sensorPara[0].inSrcTypeSel >= VIRTUAL_CHANNEL_1) {
                    if(sensorPara[0].mipiPad == 0) {

                        //sensor interface control (0x8100)
                        ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, MIPI_SENSOR);
                        //sensor muxer (0x8120)
                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                            MIPI_SENSOR, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                        //virtual channel using CAM_SV(0x8900)
                        ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, VIRTUAL_CHANNEL_1);
                        ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                                            VIRTUAL_CHANNEL_1, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);
                        //Cam SV Tg Grab
                        ret = pSeninfDrv->setSV1GrabRange(0x00, (gVCInfo.VC1_SIZEH *10)>>3, 0x00, 0x01);
                        ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);
                        ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                        // Sensor interface for VC
                        MY_LOGD("VC0_ID(%d),VC0_DataType(%d),VC1_ID(%d),VC1_DataType(%d),VC2_ID(%d),VC2_DataType(%d),VC3_ID(%d),VC3_DataType(%d)",
                                 gVCInfo.VC0_ID,gVCInfo.VC0_DataType,gVCInfo.VC1_ID,gVCInfo.VC1_DataType,gVCInfo.VC2_ID,gVCInfo.VC2_DataType,
                                 gVCInfo.VC3_ID,gVCInfo.VC3_DataType);
                        ret = pSeninfDrv->setSeninf1VC((gVCInfo.VC0_DataType<<2)|(gVCInfo.VC0_ID&0x03),(gVCInfo.VC1_DataType<<2)|(gVCInfo.VC1_ID&0x03),
                                                        (gVCInfo.VC2_DataType<<2)|(gVCInfo.VC2_ID&0x03),(gVCInfo.VC3_DataType<<2)|(gVCInfo.VC3_ID&0x03),gVCInfo.VC_Num);
                        //NCSI2 control reg
                        ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);

                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_1);
                        sensorDynamicInfo[0].TgVR1Info = CAM_SV_1;
                        sensorDynamicInfo[0].TgVR2Info = CAM_TG_NONE;
                    }
                    else {
                        MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[0].mipiPad);
                    }
                }
                else if (sensorPara[0].inSrcTypeSel == MIPI_SENSOR) {
                    if(sensorPara[0].mipiPad == 0) {
                        if(sensorPara[0].MIPI_OPHY_TYPE == 1) // CSI2
                        {
                            //sensor interface control (0x8100)
                            ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, CSI2);
                            //sensor muxer (0x8120)
                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                CSI2, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);
                            ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,1,0,sensorPara[0].DPCM_TYPE);
                        }
                        else
                        {
                            ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                        }

                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                    }
                    else if(sensorPara[0].mipiPad == 1) {
                        if(sensorPara[0].MIPI_OPHY_TYPE == 1) // CSI2
                        {
                            ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, CSI2);
                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                CSI2, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);
                            ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,1,0,sensorPara[0].DPCM_TYPE);
                        }
                        else
                        {
                            ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);
                        }

                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                    }
                    else if(sensorPara[0].mipiPad == 2) {
                        if(sensorPara[0].MIPI_OPHY_TYPE == 1) // CSI2
                        {
                            ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, CSI2);
                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1, sensorPara[0].u1VsyncPol? 0 : 1,
                                CSI2, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);
                            ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,1,0,sensorPara[0].DPCM_TYPE);
                        }
                        else
                        {
                            ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                            sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,sensorPara[0].NCSI2_HSRXDET_MODE,sensorPara[0].DPCM_TYPE);

                        }
                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                    }
                    else {
                        MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[0].mipiPad);
                    }
                }
                else if (sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) {
                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf4Parallel(1, sensorPara[0].inDataType);

                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                }
                else if (sensorPara[0].inSrcTypeSel == SERIAL_SENSOR) {
                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf4Scam(1,sensorPara[0].SCAM_CLK_INV,sensorPara[0].cropWidth,sensorPara[0].cropHeight,1,sensorPara[0].SCAM_DataNumber,sensorPara[0].SCAM_DDR_En);

                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                }
                else if (sensorPara[0].inSrcTypeSel == TEST_MODEL) {
                   ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                   ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                   ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                }
                else {
                    MY_LOGE("[seninfControl]main type = %d not support \n",sensorPara[0].inSrcTypeSel);
                }
                sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                break;

            case SENSOR_DEV_SUB:
                if(meSensorDev != currSensorDev) {
                    MY_LOGE("seninfControl current control not match. meSensorDev = %d, currSensorDev = %d\n",meSensorDev,currSensorDev);
                }

                ret = pSeninfDrv->setSeninf1NCSI2(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);    // Disable main's NCSI first
                ret = pSeninfDrv->setSeninf2NCSI2(0, 0, 0, 0, 0, 0, 0, 0, 0, 0);   // Disable sub's NCSI first
                ret = pSeninfDrv->setTg1ViewFinderMode(0); //Disable tg1
                ret = pSeninfDrv->setTg2ViewFinderMode(0); //Disable tg2
#ifndef SUB_TG2 // Sub use TG1
                ret = pSeninfDrv->setTg1GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);


                ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                        sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );
#else // Sub use TG2
                 ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);


                ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                        sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );
#endif

                if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {
                    if(sensorPara[1].mipiPad == 2) {
                        //sensorPara[1].MIPI_OPHY_TYPE=1; //For Test only
                        if(sensorPara[1].MIPI_OPHY_TYPE == 1) // CSI2
                        {
                            //sensor interface control (0x8900)
                            ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, CSI2);
                            //sensor muxer (0x8120)
                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                CSI2, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);
                            ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,1,0,sensorPara[1].DPCM_TYPE);
                        }
                        else
                        {
                            ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                            sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                        }

#ifndef SUB_TG2 // Sub use TG1
                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
#else // Sub use TG2
                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
#endif
                    }
                    else if(sensorPara[1].mipiPad == 1) {
                        if(sensorPara[1].MIPI_OPHY_TYPE == 1) // CSI2
                        {
                            ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, CSI2);
                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                CSI2, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);
                            ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,1,0,sensorPara[1].DPCM_TYPE);
                        }
                        else
                        {
                            ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                            sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                        }
#ifndef SUB_TG2 // Sub use TG1
                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
#else // Sub use TG2
                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
#endif
                    }
                    else if(sensorPara[1].mipiPad == 0) {
                        if(sensorPara[1].MIPI_OPHY_TYPE == 1) // CSI2
                        {
                            ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, CSI2);
                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1, sensorPara[1].u1VsyncPol? 0 : 1,
                                CSI2, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);
                            ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,1,0,sensorPara[1].DPCM_TYPE);
                        }
                        else
                        {
                            ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                            //no virtual channel
                            ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                            sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[1].DPCM_TYPE);
                        }
#ifndef SUB_TG2 // Sub use TG1
                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
#else // Sub use TG2
                       ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
#endif
                    }
                    else {
                        MY_LOGE("Sub camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                    }

                }
                else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

#ifndef SUB_TG2 // Sub use TG1
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
#else // Sub use TG2
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
#endif
                }
                else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf4Scam(1,sensorPara[1].SCAM_CLK_INV,sensorPara[1].cropWidth,sensorPara[1].cropHeight,1,sensorPara[1].SCAM_DataNumber,sensorPara[1].SCAM_DDR_En);

#ifndef SUB_TG2 // Sub use TG1
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
#else // Sub use TG2
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
#endif
                }
                else if (sensorPara[0].inSrcTypeSel == TEST_MODEL) {
                   ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                   ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                   ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                }
                else {
                    MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                }
#ifndef SUB_TG2 // Sub use TG1
                sensorDynamicInfo[1].TgInfo = CAM_TG_1;
#else // Sub use TG2
                sensorDynamicInfo[1].TgInfo = CAM_TG_2;
#endif
                break;

                case SENSOR_DEV_MAIN_2:
                    if(meSensorDev != currSensorDev) {
                        MY_LOGE("seninfControl current control not match. meSensorDev = %d, currSensorDev = %d\n",meSensorDev,currSensorDev);
                    }

                    ret = pSeninfDrv->setTg1GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

                    ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);


                    ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                            sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                    ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                    if (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR) {
                        if(sensorPara[2].mipiPad == 0) {
                            if(sensorPara[2].MIPI_OPHY_TYPE == 1) // CSI2
                            {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, CSI2);
                                ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                    CSI2, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,1,0,sensorPara[2].DPCM_TYPE);
                            }
                            else
                            {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                            }
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                        }
                        else if(sensorPara[2].mipiPad == 1) {
                            if(sensorPara[2].MIPI_OPHY_TYPE == 1) // CSI2
                            {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, CSI2);
                                ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                    CSI2, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,1,0,sensorPara[2].DPCM_TYPE);
                            }
                            else
                            {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                            }
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                        }
                        else if(sensorPara[2].mipiPad == 2) {
                            if(sensorPara[2].MIPI_OPHY_TYPE == 1) // CSI2
                            {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, CSI2);
                                ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1, sensorPara[2].u1VsyncPol? 0 : 1,
                                    CSI2, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,1,0,sensorPara[2].DPCM_TYPE);
                            }
                            else
                            {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,sensorPara[1].NCSI2_HSRXDET_MODE,sensorPara[2].DPCM_TYPE);
                            }
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                        }
                        else {
                            MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[2].mipiPad);
                        }


                    }
                    else if (sensorPara[2].inSrcTypeSel == PARALLEL_SENSOR) {
                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                        ret = pSeninfDrv->setSeninf4Parallel(1, sensorPara[0].inDataType);

                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                    }
                    else if (sensorPara[2].inSrcTypeSel == SERIAL_SENSOR) {
                        ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                        //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum);

                        ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                    }
                    else {
                        MY_LOGE("[seninfControl]main2 type = %d not support \n",sensorPara[2].inSrcTypeSel);
                    }
                    sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                    break;

            case (SENSOR_DEV_MAIN|SENSOR_DEV_SUB):
                switch (currSensorDev) {
                    case (SENSOR_DEV_MAIN|SENSOR_DEV_SUB):
                        //main
                        ret = pSeninfDrv->setTg1GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                        ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);



                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                                sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                        ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                        //sub
                        ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                        ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);



                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                        ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                        if ((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR)) {
                            if(sensorPara[0].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                            }
                            else if(sensorPara[0].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                            }
                            else if(sensorPara[0].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                            }

                            if(sensorPara[1].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                            }
                            else if(sensorPara[1].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                            }
                            else if(sensorPara[1].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                            }

                        }
                        else if ((sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR)) {
                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);

                            if(sensorPara[1].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                            }
                            else if(sensorPara[1].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                            }
                            else if(sensorPara[1].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                            }
                        }
                        else if((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR)) {
                            if(sensorPara[0].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                            }
                            else if(sensorPara[0].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                            }
                            else if(sensorPara[0].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                            }

                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                        }
                        else {
                            MY_LOGE("[seninfControl]main type = %d,sub type = %d not support \n",sensorPara[0].inSrcTypeSel,sensorPara[1].inSrcTypeSel);
                        }

                        sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                        sensorDynamicInfo[1].TgInfo = CAM_TG_2;

                        break;

                    case SENSOR_DEV_MAIN:
                        MY_LOGD("seninfControl setting main\n");
                        if ((sensorDynamicInfo[1].TgInfo & CAM_TG_2) || (sensorDynamicInfo[1].TgInfo == CAM_TG_NONE)) {
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                                    sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                            if (sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) {
                                if(sensorPara[0].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else if(sensorPara[0].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[0].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                                else {
                                    MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[0].mipiPad);
                                }

                            }
                            else if (sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]main type = %d not support \n",sensorPara[0].inSrcTypeSel);
                            }
                            sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                        }
                        else if ( sensorDynamicInfo[1].TgInfo & CAM_TG_1) {
                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                                    sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                            ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                            if (sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) {
                                if(sensorPara[0].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,0);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                                else if(sensorPara[0].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,0);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[0].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                                }
                                else {
                                    MY_LOGE("Main camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[0].mipiPad);
                                }

                            }
                            else if (sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]main type = %d not support \n",sensorPara[0].inSrcTypeSel);
                            }
                            sensorDynamicInfo[0].TgInfo = CAM_TG_2;

                        }
                        break;

                    case SENSOR_DEV_SUB:
                        MY_LOGD("seninfControl setting sub\n");
                        if ( (sensorDynamicInfo[0].TgInfo & CAM_TG_1) || (sensorDynamicInfo[0].TgInfo == CAM_TG_NONE)) {

                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                    sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );
#ifndef USING_MTK_LDVT

                            if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {
                                if(sensorPara[1].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                                }
                                else if(sensorPara[1].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[1].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                                else {
                                    MY_LOGE("Sub camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                                }
                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }
#else

                            if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                //no virtual channel
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);

                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }

#endif
                            sensorDynamicInfo[1].TgInfo = CAM_TG_2;
                        }
                        else if ( sensorDynamicInfo[0].TgInfo & CAM_TG_2) {
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                                    sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                            if (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) {
                                if(sensorPara[1].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                                else if(sensorPara[1].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[1].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                    //no virtual channel
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,
                                    sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder,0,0,sensorPara[1].DPCM_TYPE);

                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else {
                                    MY_LOGE("Sub camera mipi pad(%d) incorrect, please check cfg_setting_imgsensor.cpp",sensorPara[1].mipiPad);
                                }
                            }
                            else if (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[1].inSrcTypeSel == TEST_MODEL) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                ret = pSeninfDrv->setTG1_TM_Ctl(SENINF_1,1,sensorTMPara.TM_DummyPixel,sensorTMPara.TM_Vsync,sensorTMPara.TM_Line,sensorTMPara.TM_Pixel);
                            }
                            else {
                                MY_LOGE("[seninfControl]sub type = %d not support \n",sensorPara[1].inSrcTypeSel);
                            }

                            sensorDynamicInfo[1].TgInfo = CAM_TG_1;
                        }
                        break;

                    default:
                        MY_LOGE("seninfControl mismatch error,  meSensorDev = %d, currSensorDev = %d\n",meSensorDev,currSensorDev);
                        break;
                }

                break;

            case (SENSOR_DEV_MAIN|SENSOR_DEV_MAIN_2):
                switch (currSensorDev) {
                    case (SENSOR_DEV_MAIN|SENSOR_DEV_MAIN_2):
                        //main
                        ret = pSeninfDrv->setTg1GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                        ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                        ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                                sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                        ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                        //main2
                        ret = pSeninfDrv->setTg2GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

                        ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);


                        ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                                sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                        ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );


                        if ((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR)) {


                            if(sensorPara[0].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                            }
                            else if(sensorPara[0].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                            }
                            else if(sensorPara[0].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                            }

                            if(sensorPara[2].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                            }
                            else if(sensorPara[2].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                            }
                            else if(sensorPara[2].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                            }

                        }
                        else if ((sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) && (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR)) {
                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);

                            if(sensorPara[2].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                            }
                            else if(sensorPara[2].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                            }
                            else if(sensorPara[2].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                    sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                            }
                        }
                        else if((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR)) {
                            ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                            ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[2].inDataType);
                            ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);

                            if(sensorPara[0].mipiPad == 0) {
                                ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                            }
                            else if(sensorPara[0].mipiPad == 1) {
                                ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                            }
                            else if(sensorPara[0].mipiPad == 2) {
                                ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                    sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                            }
                        }
                        else {
                            MY_LOGE("[seninfControl]main type = %d,main2 type = %d not support \n",sensorPara[0].inSrcTypeSel,sensorPara[2].inSrcTypeSel);
                        }



                        sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                        sensorDynamicInfo[2].TgInfo = CAM_TG_2;

                        break;

                    case SENSOR_DEV_MAIN:
                        MY_LOGD("seninfControl setting main\n");
                        if ( (sensorDynamicInfo[2].TgInfo & CAM_TG_2) || (sensorDynamicInfo[2].TgInfo == CAM_TG_NONE)) {
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                                    sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                            if (sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) {
                                if(sensorPara[0].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                                else if(sensorPara[0].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[0].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                            }
                            else if (sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else {
                                MY_LOGE("[seninfControl]main type = %d not support \n",sensorPara[0].inSrcTypeSel);
                            }

                            sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                        }
                        else if ( sensorDynamicInfo[2].TgInfo & CAM_TG_1) {
                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                                    sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                            ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                            if (sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) {
                                if(sensorPara[0].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                                else if(sensorPara[0].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[0].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,
                                        sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder,0,0,sensorPara[0].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                                }

                            }
                            else if (sensorPara[0].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[0].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else {
                                MY_LOGE("[seninfControl]main type = %d not support \n",sensorPara[0].inSrcTypeSel);
                            }

                            sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                        }

                        break;

                    case SENSOR_DEV_MAIN_2:
                        //main2
                        if ( (sensorDynamicInfo[0].TgInfo & CAM_TG_1) || (sensorDynamicInfo[0].TgInfo == CAM_TG_NONE)) {
                            ret = pSeninfDrv->setTg2GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

                            ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);


                            ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                                    sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                            ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                            if (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR) {
                                if(sensorPara[2].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_2);
                                }
                                else if(sensorPara[2].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                                }
                                else if(sensorPara[2].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay
                                        ,sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_1);
                                }
                            }
                            else if (sensorPara[2].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[2].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else if (sensorPara[2].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                            }
                            else {
                                MY_LOGE("[seninfControl]main type = %d not support \n",sensorPara[2].inSrcTypeSel);
                            }

                            sensorDynamicInfo[2].TgInfo = CAM_TG_2;
                        }
                        else if ( sensorDynamicInfo[0].TgInfo & CAM_TG_2) {
                            ret = pSeninfDrv->setTg1GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

                            ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);


                            ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                                    sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                            ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );

                            if (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR) {
                                if(sensorPara[2].mipiPad == 1) {
                                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_2);
                                }
                                else if(sensorPara[2].mipiPad == 2) {
                                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_3);
                                }
                                else if(sensorPara[2].mipiPad == 0) {
                                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,
                                        sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder,0,0,sensorPara[2].DPCM_TYPE);
                                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                                }
                            }
                            else if (sensorPara[2].inSrcTypeSel == PARALLEL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[2].inDataType);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else if (sensorPara[2].inSrcTypeSel == SERIAL_SENSOR) {
                                ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                                //ret = pSeninfDrv->setSeninf4Scam(unsigned int scamEn,unsigned int clkInv,unsigned int width,unsigned int height,unsigned int contiMode,unsigned int csdNum);

                                ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                            }
                            else {
                                MY_LOGE("[seninfControl]main type = %d not support \n",sensorPara[2].inSrcTypeSel);
                            }

                            sensorDynamicInfo[2].TgInfo = CAM_TG_1;

                        }
                        break;

                    default:
                        MY_LOGE("seninfControl mismatch error,  meSensorDev = %d, currSensorDev = %d\n",meSensorDev,currSensorDev);
                        break;

                }
                break;
            #if 0
            case (SENSOR_DEV_MAIN|SENSOR_DEV_SUB|SENSOR_DEV_MAIN_2)://should not support this case
                //main
                ret = pSeninfDrv->setTg1GrabRange(sensorPara[0].u4PixelX0, sensorPara[0].u4PixelX1, sensorPara[0].u4PixelY0, sensorPara[0].u4PixelY1);

                ret = pSeninfDrv->setTg1Cfg((TG_FORMAT_ENUM)sensorPara[0].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[0].senInLsb, sensorDynamicInfo[0].pixelMode);


                ret = pSeninfDrv->setSeninf1MuxCtrl(sensorPara[0].u1HsyncPol?0 : 1,
                                                        sensorPara[0].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[0].inDataType, sensorDynamicInfo[0].pixelMode);

                ret = pSeninfDrv->setTg1ViewFinderMode(sensorPara[0].u1IsContinuous? 0 : 1 );

                //sub
                ret = pSeninfDrv->setTg2GrabRange(sensorPara[1].u4PixelX0, sensorPara[1].u4PixelX1, sensorPara[1].u4PixelY0, sensorPara[1].u4PixelY1);

                ret = pSeninfDrv->setTg2Cfg((TG_FORMAT_ENUM)sensorPara[1].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[1].senInLsb, sensorDynamicInfo[1].pixelMode);


                ret = pSeninfDrv->setSeninf2MuxCtrl(sensorPara[1].u1HsyncPol?0 : 1,
                                                        sensorPara[1].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[1].inDataType, sensorDynamicInfo[1].pixelMode);

                ret = pSeninfDrv->setTg2ViewFinderMode(sensorPara[1].u1IsContinuous? 0 : 1 );

                //main2
                ret = pSeninfDrv->setSV1GrabRange(sensorPara[2].u4PixelX0, sensorPara[2].u4PixelX1, sensorPara[2].u4PixelY0, sensorPara[2].u4PixelY1);

                ret = pSeninfDrv->setSV1Cfg((TG_FORMAT_ENUM)sensorPara[2].inDataType, (SENSOR_DATA_BITS_ENUM)sensorPara[2].senInLsb, sensorDynamicInfo[2].pixelMode);


                ret = pSeninfDrv->setSeninf3MuxCtrl(sensorPara[2].u1HsyncPol?0 : 1,
                                                        sensorPara[2].u1VsyncPol? 0 : 1,(SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel, (TG_FORMAT_ENUM)sensorPara[2].inDataType, sensorDynamicInfo[2].pixelMode);

                ret = pSeninfDrv->setSV1ViewFinderMode(sensorPara[2].u1IsContinuous? 0 : 1 );



                if ((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR)) {
                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_2);

                }
                else if ((sensorPara[0].inSrcTypeSel== PARALLEL_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR)) {
                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[0].inDataType);
                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_4);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_2);
                }
                else if ((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel == PARALLEL_SENSOR) && (sensorPara[2].inSrcTypeSel >= MIPI_SENSOR)) {
                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf2Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[1].inDataType);
                    ret = pSeninfDrv->setSeninf2NCSI2(sensorPara[2].u1MIPIDataTermDelay,sensorPara[2].u1MIPIDataSettleDelay,sensorPara[2].u1MIPIClkTermDelay,sensorPara[2].u1VsyncPol,sensorPara[2].u1MIPILaneNum,1,sensorPara[2].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_4);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_2);
                }
                else if ((sensorPara[0].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[1].inSrcTypeSel >= MIPI_SENSOR) && (sensorPara[2].inSrcTypeSel == PARALLEL_SENSOR)) {
                    ret = pSeninfDrv->setSeninf1Ctrl((PAD2CAM_DATA_ENUM)sensorPara[0].padSel, (SENINF_SOURCE_ENUM)sensorPara[0].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf3Ctrl((PAD2CAM_DATA_ENUM)sensorPara[1].padSel, (SENINF_SOURCE_ENUM)sensorPara[1].inSrcTypeSel);
                    ret = pSeninfDrv->setSeninf4Ctrl((PAD2CAM_DATA_ENUM)sensorPara[2].padSel, (SENINF_SOURCE_ENUM)sensorPara[2].inSrcTypeSel);

                    ret = pSeninfDrv->setSeninf1NCSI2(sensorPara[0].u1MIPIDataTermDelay,sensorPara[0].u1MIPIDataSettleDelay,sensorPara[0].u1MIPIClkTermDelay,sensorPara[0].u1VsyncPol,sensorPara[0].u1MIPILaneNum,1,sensorPara[0].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf3NCSI2(sensorPara[1].u1MIPIDataTermDelay,sensorPara[1].u1MIPIDataSettleDelay,sensorPara[1].u1MIPIClkTermDelay,sensorPara[1].u1VsyncPol,sensorPara[1].u1MIPILaneNum,1,sensorPara[1].u1MIPIPacketECCOrder);
                    ret = pSeninfDrv->setSeninf4Parallel(1,sensorPara[2].inDataType);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG1,SENINF_1);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_TG2,SENINF_3);
                    ret = pSeninfDrv->setSeninfTopMuxCtrl(SENINF_TOP_SV1,SENINF_4);
                }
                else {
                    MY_LOGE("[seninfControl]main type = %d,sub type  = %d, main2 type = %d not support \n",sensorPara[0].inSrcTypeSel,sensorPara[1].inSrcTypeSel,sensorPara[2].inSrcTypeSel);
                }
                sensorDynamicInfo[0].TgInfo = CAM_TG_1;
                sensorDynamicInfo[1].TgInfo = CAM_TG_2;
                sensorDynamicInfo[2].TgInfo = CAM_SV_1;
                break;
            #endif
            default:
                MY_LOGE("Not support, meSensorDev = 0x%x",meSensorDev);
                break;
         MY_LOGD("Tg usage infomation: Main = %d, Sub = %d, Main_2 =%d\n",sensorDynamicInfo[0].TgInfo,sensorDynamicInfo[1].TgInfo,sensorDynamicInfo[2].TgInfo);
        }
    }
    else {

        // ret = pSeninfDrv->setSeninf1NCSI2(0, 0, 0, 0, 0, 0, 0);
        // ret = pSeninfDrv->setSeninf2NCSI2(0, 0, 0, 0, 0, 0, 0);
        // ret = pSeninfDrv->setSeninf3NCSI2(0, 0, 0, 0, 0, 0, 0);
        // ret = pSeninfDrv->setSeninf4Scam(0, 0, 0, 0, 0, 0);
        // ret = pSeninfDrv->setSeninf4Parallel(0);
    }

    pSeninfDrv->destroyInstance();

    return ret;

}



/*******************************************************************************
*
********************************************************************************/
inline void setDebugTag(DEBUG_SENSOR_INFO_S &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = CAMTAG(DEBUG_CAM_SENSOR_MID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 HalSensor::setDebugInfo(IBaseCamExif *pIBaseCamExif)
{

    DEBUG_SENSOR_INFO_T sensorDebugInfo;
    MUINT32 exifId;
    MINT32 ret = 0;

    //Exif debug info
#ifndef  USING_MTK_LDVT

        setDebugTag(sensorDebugInfo, SENSOR_TAG_VERSION, (MUINT32)SENSOR_DEBUG_TAG_VERSION);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_COLORORDER, (MUINT32)sensorStaticInfo[0].sensorFormatOrder);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_DATATYPE, (MUINT32)sensorRawInfo[0].sensorType);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_HARDWARE_INTERFACE, (MUINT32)sensorPara[0].inSrcTypeSel);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_START_X, (MUINT32)sensorPara[0].u4PixelX0);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_START_Y, (MUINT32)sensorPara[0].u4PixelY0);
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_WIDTH, (MUINT32)(sensorPara[0].u4PixelX1-sensorPara[0].u4PixelX0));
        setDebugTag(sensorDebugInfo, SENSOR1_TAG_GRAB_HEIGHT, (MUINT32)(sensorPara[0].u4PixelY1-sensorPara[0].u4PixelY0));
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_COLORORDER, (MUINT32)sensorStaticInfo[1].sensorFormatOrder);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_DATATYPE, (MUINT32)sensorRawInfo[1].sensorType);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_HARDWARE_INTERFACE, (MUINT32)sensorPara[1].inSrcTypeSel);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_START_X, (MUINT32)sensorPara[1].u4PixelX0);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_START_Y, (MUINT32)sensorPara[1].u4PixelY0);
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_WIDTH, (MUINT32)(sensorPara[1].u4PixelX1-sensorPara[1].u4PixelX0));
        setDebugTag(sensorDebugInfo, SENSOR2_TAG_GRAB_HEIGHT, (MUINT32)(sensorPara[1].u4PixelY1-sensorPara[1].u4PixelY0));
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_COLORORDER, (MUINT32)sensorStaticInfo[2].sensorFormatOrder);
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_DATATYPE, (MUINT32)sensorRawInfo[2].sensorType);
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_HARDWARE_INTERFACE, (MUINT32)sensorPara[2].inSrcTypeSel);
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_GRAB_START_X, (MUINT32)sensorPara[2].u4PixelX0);
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_GRAB_START_Y, (MUINT32)sensorPara[2].u4PixelY0);
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_GRAB_WIDTH, (MUINT32)(sensorPara[2].u4PixelX1-sensorPara[2].u4PixelX0));
        setDebugTag(sensorDebugInfo, SENSOR3_TAG_GRAB_HEIGHT, (MUINT32)(sensorPara[2].u4PixelY1-sensorPara[2].u4PixelY0));


        ret = pIBaseCamExif->sendCommand(CMD_REGISTER, DEBUG_CAM_SENSOR_MID, (MUINTPTR)(&exifId));
        ret = pIBaseCamExif->sendCommand(CMD_SET_DBG_EXIF, exifId, (MUINTPTR)(&sensorDebugInfo), sizeof(DEBUG_SENSOR_INFO_T));

#endif

    return ret;
}


MINT32 HalSensor::releaseI2CTrigLock()
{
    MINT32 ret;
    SensorDrv *const pSensorDrv = SensorDrv::get();

    ret = pSensorDrv->releaseI2CTriggerLock();
    return ret;
}



