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

#define LOG_TAG "MtkCam/HalSensorList"
//
#include "MyUtils.h"
#include <mtkcam/metadata/IMetadata.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#ifdef USING_MTK_LDVT
#include "uvvf.h"
#endif

//
//
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

extern SENSOR_CONFIG_STRUCT sensorPara[3];

SensorStaticInfo sensorStaticInfo[3];//0:main, 1:sub, 2:3d
SENSORDRV_INFO_STRUCT sensorDrvInfo[3];//6593, 0:main, 1:sub, 2:main2
SENSOR_HAL_RAW_INFO_STRUCT sensorRawInfo[3];
IMetadata  gSensorMetadata[3];//main/sub/main2


/******************************************************************************
 *
 ******************************************************************************/
MVOID HalSensorList::querySensorDrvInfo()
{

    MUINT i = 0, ret = 0;
    MINT sensorDev;

    SensorDrv *const pSensorDrv = SensorDrv::get();

#ifdef MTK_MAIN2_IMGSENSOR
    for (sensorDev = SENSOR_MAIN; sensorDev <= SENSOR_MAIN_2; sensorDev <<= 1) {
#else
   #ifdef MTK_SUB_IMGSENSOR
    for (sensorDev = SENSOR_MAIN; sensorDev <= SENSOR_SUB; sensorDev <<= 1) {
   #else
    for (sensorDev = SENSOR_MAIN; sensorDev < SENSOR_SUB; sensorDev <<= 1) {
   #endif
#endif

        pSensorDrv->init((SENSOR_DEV_ENUM)sensorDev);

        pSensorDrv->getInfo2((SENSOR_DEV_ENUM)sensorDev,&sensorDrvInfo[i]);

        ret = getRawInfo((SENSOR_DEV_ENUM)sensorDev, &sensorRawInfo[i]);

        i++;
        pSensorDrv->uninit();
    }
    querySensorInfo();//to remap data


}


MUINT HalSensorList::getRawInfo(SENSOR_DEV_ENUM sensorDevId, SENSOR_HAL_RAW_INFO_STRUCT *pInfo)
{
    MUINT32 ret = 0;

    SensorDrv *const pSensorDrv = SensorDrv::get();

    switch(sensorDevId) {
        case SENSOR_MAIN:
            pInfo->u1Order = sensorDrvInfo[0].SensorOutputDataFormat;
            break;
        case SENSOR_SUB:
            pInfo->u1Order = sensorDrvInfo[1].SensorOutputDataFormat;
            break;
        case SENSOR_MAIN_2:
            pInfo->u1Order = sensorDrvInfo[2].SensorOutputDataFormat;
            break;
        default:
            break;
    }

    /* Modify getSensorType from getInfo Ioctl to directly compute sensorType from pInfo->u1Order
    //sensorType = pSensorDrv->getCurrentSensorType(sensorDevId);
    */

    // Compute sensorType from SensorOutputDataFormat
    if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_RAW_B &&
         pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_RAW_R) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_RAW;
    }
    else if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_RAW8_B &&
         pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_RAW8_R) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_RAW8;
    }
    else if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_UYVY &&
                pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_YVYU) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_YUV;
    }
    else if (pInfo->u1Order >= SENSOR_OUTPUT_FORMAT_CbYCrY &&
                pInfo->u1Order <= SENSOR_OUTPUT_FORMAT_YCrYCb) {
        pInfo->sensorType = IMAGE_SENSOR_TYPE_YCBCR;
    }
    else {
        MY_LOGE("Unsupport SensorOutputDataFormat \n");
    }

    MY_LOGD("SensorOutputDataFormat: %d, ImageSensor Type: %d\n", pInfo->u1Order, pInfo->sensorType);


    switch (pInfo->sensorType) {
        case IMAGE_SENSOR_TYPE_RAW:
            pInfo->u4BitDepth = 10;
            pInfo->u4IsPacked = 1;
            break;
        case IMAGE_SENSOR_TYPE_RAW8:
            pInfo->u4BitDepth = 8;
            pInfo->u4IsPacked = 1;
            break;
        case IMAGE_SENSOR_TYPE_RAW12:
            pInfo->u4BitDepth = 12;
            pInfo->u4IsPacked = 1;
            break;
        case IMAGE_SENSOR_TYPE_RAW14:
            pInfo->u4BitDepth = 14;
            pInfo->u4IsPacked = 1;
            break;

        case IMAGE_SENSOR_TYPE_YUV:
        case IMAGE_SENSOR_TYPE_YCBCR:
            pInfo->u4BitDepth = 8;
            pInfo->u4IsPacked = 0;
            break;
        case IMAGE_SENSOR_TYPE_RGB565:
            pInfo->u4BitDepth = 8;
            pInfo->u4IsPacked = 0;
            break;
        default:
            MY_LOGE("Unsupport Sensor Type \n");
            break;
    }


    return ret;

}



MVOID HalSensorList::querySensorInfo()
{
    MUINT idx;
    //MUINT32 data1,data2;
    MUINTPTR data1,data2;

    SensorDrv *const pSensorDrv = SensorDrv::get();

#ifdef MTK_MAIN2_IMGSENSOR
    for(idx=0; idx<3; idx++) {
#else
   #ifdef MTK_SUB_IMGSENSOR
    for(idx=0; idx<2; idx++) {
   #else
    for(idx=0; idx<1; idx++) {
   #endif
#endif
        switch (idx) {
            case 0:
                pSensorDrv->init(SENSOR_MAIN);
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_MCLK_CONNECTION, (MUINTPTR)&data1);
                sensorPara[idx].mclkSrc = data1;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_MIPI_SENSOR_PORT, (MUINTPTR)&data1);
                sensorPara[idx].mipiPad = data1;

                sensorStaticInfo[idx].sensorDevID = pSensorDrv->getMainSensorID();
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE, (MUINTPTR)&data1);
                sensorStaticInfo[idx].orientationAngle = data1;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_SENSOR_FACING_DIRECTION, (MUINTPTR)&data1);
                sensorStaticInfo[idx].facingDirection = data1;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_FAKE_ORIENTATION, (MUINTPTR)&data1);
                sensorStaticInfo[idx].fakeOrientation = data1;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_SENSOR_VIEWANGLE, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].horizontalViewAngle =data1;
                sensorStaticInfo[idx].verticalViewAngle = data2;

                data1 = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].previewFrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].captureFrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].videoFrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video1FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM1;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM2;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM3;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM4;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM5;
                pSensorDrv->sendCommand(SENSOR_MAIN, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                pSensorDrv->uninit();
                break;
            case 1:
                pSensorDrv->init(SENSOR_SUB);
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_MCLK_CONNECTION, (MUINTPTR)&data1);
                sensorPara[idx].mclkSrc = data1;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_MIPI_SENSOR_PORT, (MUINTPTR)&data1);
                sensorPara[idx].mipiPad = data1;

                sensorStaticInfo[idx].sensorDevID = pSensorDrv->getSubSensorID();
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE, (MUINTPTR)&data1);
                sensorStaticInfo[idx].orientationAngle = data1;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_SENSOR_FACING_DIRECTION, (MUINTPTR)&data1);
                sensorStaticInfo[idx].facingDirection = data1;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_FAKE_ORIENTATION, (MUINTPTR)&data1);
                sensorStaticInfo[idx].fakeOrientation = data1;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_SENSOR_VIEWANGLE, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].horizontalViewAngle =data1;
                sensorStaticInfo[idx].verticalViewAngle = data2;

                data1 = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].previewFrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].captureFrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].videoFrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video1FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM1;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM2;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM3;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM4;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM5;
                pSensorDrv->sendCommand(SENSOR_SUB, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                pSensorDrv->uninit();
                break;
            case 2:
                pSensorDrv->init(SENSOR_MAIN_2);
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_MCLK_CONNECTION, (MUINTPTR)&data1);
                sensorPara[idx].mclkSrc = data1;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_MIPI_SENSOR_PORT, (MUINTPTR)&data1);
                sensorPara[idx].mipiPad = data1;

                sensorStaticInfo[idx].sensorDevID = pSensorDrv->getMain2SensorID();
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_SENSOR_ORIENTATION_ANGLE, (MUINTPTR)&data1);
                sensorStaticInfo[idx].orientationAngle = data1;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_SENSOR_FACING_DIRECTION, (MUINTPTR)&data1);
                sensorStaticInfo[idx].facingDirection = data1;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_FAKE_ORIENTATION, (MUINTPTR)&data1);
                sensorStaticInfo[idx].fakeOrientation = data1;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_SENSOR_VIEWANGLE, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].horizontalViewAngle =data1;
                sensorStaticInfo[idx].verticalViewAngle = data2;

                data1 = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].previewFrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].captureFrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].videoFrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video1FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM1;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM2;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM3;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM4;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;
                data1 = SENSOR_SCENARIO_ID_CUSTOM5;
                pSensorDrv->sendCommand(SENSOR_MAIN_2, CMD_SENSOR_GET_DEFAULT_FRAME_RATE_BY_SCENARIO, (MUINTPTR)&data1, (MUINTPTR)&data2);
                sensorStaticInfo[idx].video2FrameRate = data2;

                pSensorDrv->uninit();
                break;
            default:
                break;
        }

        switch (sensorRawInfo[idx].sensorType) {
            case IMAGE_SENSOR_TYPE_RAW:
                sensorStaticInfo[idx].sensorType = SENSOR_TYPE_RAW;
                sensorStaticInfo[idx].rawSensorBit = RAW_SENSOR_10BIT;
                break;
            case IMAGE_SENSOR_TYPE_RAW8:
                sensorStaticInfo[idx].sensorType = SENSOR_TYPE_RAW;
                sensorStaticInfo[idx].rawSensorBit = RAW_SENSOR_8BIT;
                break;

            case IMAGE_SENSOR_TYPE_RAW12:
                sensorStaticInfo[idx].sensorType = SENSOR_TYPE_RAW;
                sensorStaticInfo[idx].rawSensorBit = RAW_SENSOR_12BIT;
                break;

            case IMAGE_SENSOR_TYPE_RAW14:
                sensorStaticInfo[idx].sensorType = SENSOR_TYPE_RAW;
                sensorStaticInfo[idx].rawSensorBit = RAW_SENSOR_14BIT;
                break;
            case IMAGE_SENSOR_TYPE_YUV:
            case IMAGE_SENSOR_TYPE_YCBCR:
                sensorStaticInfo[idx].sensorType = SENSOR_TYPE_YUV;
                sensorStaticInfo[idx].rawSensorBit = RAW_SENSOR_ERROR;
                break;
            case IMAGE_SENSOR_TYPE_RGB565:
                sensorStaticInfo[idx].sensorType = SENSOR_TYPE_RGB;
                sensorStaticInfo[idx].rawSensorBit = RAW_SENSOR_ERROR;
                break;
            case IMAGE_SENSOR_TYPE_JPEG:
                sensorStaticInfo[idx].sensorType = SENSOR_TYPE_JPEG;
                sensorStaticInfo[idx].rawSensorBit = RAW_SENSOR_ERROR;
                break;
            default:
                sensorStaticInfo[idx].sensorType = SENSOR_TYPE_UNKNOWN;
                sensorStaticInfo[idx].rawSensorBit = RAW_SENSOR_ERROR;
                break;
        }

         switch (sensorRawInfo[idx].u1Order) {
             case SENSOR_OUTPUT_FORMAT_RAW_B:
             case SENSOR_OUTPUT_FORMAT_RAW8_B:
                sensorStaticInfo[idx].sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_B;
                break;
             case SENSOR_OUTPUT_FORMAT_RAW_Gb:
             case SENSOR_OUTPUT_FORMAT_RAW8_Gb:
                sensorStaticInfo[idx].sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gb;
                break;
             case SENSOR_OUTPUT_FORMAT_RAW_Gr:
             case SENSOR_OUTPUT_FORMAT_RAW8_Gr:
                sensorStaticInfo[idx].sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_Gr;
                break;
             case SENSOR_OUTPUT_FORMAT_RAW_R:
             case SENSOR_OUTPUT_FORMAT_RAW8_R:
                sensorStaticInfo[idx].sensorFormatOrder = SENSOR_FORMAT_ORDER_RAW_R;
                break;
             case SENSOR_OUTPUT_FORMAT_UYVY:
             case SENSOR_OUTPUT_FORMAT_CbYCrY:
                sensorStaticInfo[idx].sensorFormatOrder = SENSOR_FORMAT_ORDER_UYVY;
                break;
             case SENSOR_OUTPUT_FORMAT_VYUY:
             case SENSOR_OUTPUT_FORMAT_CrYCbY:
                sensorStaticInfo[idx].sensorFormatOrder = SENSOR_FORMAT_ORDER_VYUY;
                break;
             case SENSOR_OUTPUT_FORMAT_YUYV:
             case SENSOR_OUTPUT_FORMAT_YCbYCr:
                sensorStaticInfo[idx].sensorFormatOrder = SENSOR_FORMAT_ORDER_YUYV;
                break;
             case SENSOR_OUTPUT_FORMAT_YVYU:
             case SENSOR_OUTPUT_FORMAT_YCrYCb:
                sensorStaticInfo[idx].sensorFormatOrder = SENSOR_FORMAT_ORDER_YVYU;
                break;
             default:
                sensorStaticInfo[idx].sensorFormatOrder = SENSOR_FORMAT_ORDER_NONE;
                break;

         }



        sensorStaticInfo[idx].iHDRSupport = sensorDrvInfo[idx].iHDRSupport;
        sensorStaticInfo[idx].PDAF_Support = sensorDrvInfo[idx].PDAF_Support;
        sensorStaticInfo[idx].previewWidth = sensorDrvInfo[idx].SensorPreviewWidth;
        sensorStaticInfo[idx].previewHeight = sensorDrvInfo[idx].SensorPreviewHeight;
        sensorStaticInfo[idx].captureWidth = sensorDrvInfo[idx].SensorCapWidth;
        sensorStaticInfo[idx].captureHeight = sensorDrvInfo[idx].SensorCapHeight;
        sensorStaticInfo[idx].videoWidth = sensorDrvInfo[idx].SensorVideoWidth;
        sensorStaticInfo[idx].videoHeight = sensorDrvInfo[idx].SensorVideoHeight;
        sensorStaticInfo[idx].video1Width = sensorDrvInfo[idx].SensorVideo1Width;
        sensorStaticInfo[idx].video1Height = sensorDrvInfo[idx].SensorVideo1Height;
        sensorStaticInfo[idx].video2Width = sensorDrvInfo[idx].SensorVideo2Width;
        sensorStaticInfo[idx].video2Height = sensorDrvInfo[idx].SensorVideo2Height;
        sensorStaticInfo[idx].SensorCustom1Width = sensorDrvInfo[idx].SensorCustom1Width;
        sensorStaticInfo[idx].SensorCustom1Height = sensorDrvInfo[idx].SensorCustom1Height;
        sensorStaticInfo[idx].SensorCustom2Width = sensorDrvInfo[idx].SensorCustom2Width;
        sensorStaticInfo[idx].SensorCustom2Height = sensorDrvInfo[idx].SensorCustom2Height;
        sensorStaticInfo[idx].SensorCustom3Width = sensorDrvInfo[idx].SensorCustom3Width;
        sensorStaticInfo[idx].SensorCustom3Height = sensorDrvInfo[idx].SensorCustom3Height;
        sensorStaticInfo[idx].SensorCustom4Width = sensorDrvInfo[idx].SensorCustom4Width;
        sensorStaticInfo[idx].SensorCustom4Height = sensorDrvInfo[idx].SensorCustom4Height;
        sensorStaticInfo[idx].SensorCustom5Width = sensorDrvInfo[idx].SensorCustom5Width;
        sensorStaticInfo[idx].SensorCustom5Height = sensorDrvInfo[idx].SensorCustom5Height;
        sensorStaticInfo[idx].previewDelayFrame = sensorDrvInfo[idx].PreviewDelayFrame;
        sensorStaticInfo[idx].captureDelayFrame = sensorDrvInfo[idx].CaptureDelayFrame;
        sensorStaticInfo[idx].videoDelayFrame = sensorDrvInfo[idx].VideoDelayFrame;
        sensorStaticInfo[idx].video1DelayFrame = sensorDrvInfo[idx].HighSpeedVideoDelayFrame;
        sensorStaticInfo[idx].video2DelayFrame = sensorDrvInfo[idx].SlimVideoDelayFrame;
        sensorStaticInfo[idx].Custom1DelayFrame = sensorDrvInfo[idx].Custom1DelayFrame;
        sensorStaticInfo[idx].Custom2DelayFrame = sensorDrvInfo[idx].Custom2DelayFrame;
        sensorStaticInfo[idx].Custom3DelayFrame = sensorDrvInfo[idx].Custom3DelayFrame;
        sensorStaticInfo[idx].Custom4DelayFrame = sensorDrvInfo[idx].Custom4DelayFrame;
        sensorStaticInfo[idx].Custom5DelayFrame = sensorDrvInfo[idx].Custom5DelayFrame;
        sensorStaticInfo[idx].aeShutDelayFrame = sensorDrvInfo[idx].AEShutDelayFrame;
        sensorStaticInfo[idx].aeSensorGainDelayFrame = sensorDrvInfo[idx].AESensorGainDelayFrame;
        sensorStaticInfo[idx].aeISPGainDelayFrame = sensorDrvInfo[idx].AEISPGainDelayFrame;


        sensorStaticInfo[idx].previewActiveWidth = sensorDrvInfo[idx].SensorEffectivePreviewWidth;//3d use onlyl?
        sensorStaticInfo[idx].previewActiveHeight = sensorDrvInfo[idx].SensorEffectivePreviewHeight;//3d use onlyl?
        sensorStaticInfo[idx].captureActiveWidth = sensorDrvInfo[idx].SensorEffectiveCapWidth;//3d use onlyl?
        sensorStaticInfo[idx].captureActiveHeight = sensorDrvInfo[idx].SensorEffectiveCapHeight;//3d use onlyl?
        sensorStaticInfo[idx].videoActiveWidth = sensorDrvInfo[idx].SensorEffectiveVideoWidth;//3d use onlyl?
        sensorStaticInfo[idx].videowActiveHeight = sensorDrvInfo[idx].SensorEffectiveVideoHeight;//3d use onlyl?
        sensorStaticInfo[idx].previewHoizontalOutputOffset = sensorDrvInfo[idx].SensorPreviewWidthOffset;//3d use onlyl?
        sensorStaticInfo[idx].previewVerticalOutputOffset = sensorDrvInfo[idx].SensorPreviewHeightOffset; //3d use onlyl?
        sensorStaticInfo[idx].captureHoizontalOutputOffset = sensorDrvInfo[idx].SensorCapWidthOffset;//3d use onlyl?
        sensorStaticInfo[idx].captureVerticalOutputOffset = sensorDrvInfo[idx].SensorCapHeightOffset; //3d use onlyl?
        sensorStaticInfo[idx].videoHoizontalOutputOffset = sensorDrvInfo[idx].SensorVideoWidthOffset;//3d use onlyl?
        sensorStaticInfo[idx].videoVerticalOutputOffset = sensorDrvInfo[idx].SensorVideoHeightOffset; //3d use onlyl?
        sensorStaticInfo[idx].virtualChannelSupport = sensorDrvInfo[idx].virtualChannelSupport;
        sensorStaticInfo[idx].iHDR_First_IS_LE = sensorDrvInfo[idx].iHDR_First_IS_LE;
        sensorStaticInfo[idx].SensorModeNum = sensorDrvInfo[idx].SensorModeNum;
    }
    MY_LOGD("MCLK Source: Main = %d, Sub = %d, Main_2 =%d\n",sensorPara[0].mclkSrc,sensorPara[1].mclkSrc,sensorPara[2].mclkSrc);
    MY_LOGD("MIPI pad: Main = %d, Sub = %d, Main_2 =%d\n",sensorPara[0].mipiPad,sensorPara[1].mipiPad,sensorPara[2].mipiPad);
}



MVOID HalSensorList::buildSensorMetadata()
{
    MINT idx;
    MINT64 exposureTime1 = 0x4000;
    MINT64 exposureTime2 = 0x4000;
    MUINT8 u8Para = 0;
    MINT32 s32Para = 0;

    MY_LOGD("impBuildSensorInfo start!\n");

#ifdef MTK_MAIN2_IMGSENSOR
    for(idx=0; idx<3; idx++) {
#else
   #ifdef MTK_SUB_IMGSENSOR
    for(idx=0; idx<2; idx++) {
   #else
    for(idx=0; idx<1; idx++) {
   #endif
#endif

        IMetadata& metadataA = gSensorMetadata[idx];

        {
    IMetadata::IEntry entryA(MTK_SENSOR_EXPOSURE_TIME);
    entryA.push_back(exposureTime1, Type2Type<MINT64>());
    entryA.push_back(exposureTime2, Type2Type<MINT64>());
            metadataA.update(MTK_SENSOR_EXPOSURE_TIME, entryA);
        }


        { //using full size?
            IMetadata::IEntry entryA(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
            MRect region1(MPoint(sensorStaticInfo[idx].captureHoizontalOutputOffset,sensorStaticInfo[idx].captureVerticalOutputOffset), MSize(sensorStaticInfo[idx].captureWidth,sensorStaticInfo[idx].captureHeight));
            entryA.push_back(region1, Type2Type<MRect>());
            metadataA.update(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, entryA);
        }

        {
            IMetadata::IEntry entryA(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT);
            switch(sensorStaticInfo[idx].sensorFormatOrder) {
                case SENSOR_FORMAT_ORDER_RAW_B:
                    u8Para = 0x3;//BGGR
                    break;
                case SENSOR_FORMAT_ORDER_RAW_Gb:
                    u8Para = 0x2;//GBRG
                    break;
                case SENSOR_FORMAT_ORDER_RAW_Gr:
                    u8Para = 0x1;//GRBG
                    break;
                case SENSOR_FORMAT_ORDER_RAW_R:
                    u8Para = 0x0;//RGGB
                    break;
                default:
                    u8Para = 0x4;//BGR not bayer
                    break;
            }
            entryA.push_back(u8Para, Type2Type<MUINT8>());
            metadataA.update(MTK_SENSOR_INFO_COLOR_FILTER_ARRANGEMENT, entryA);
        }

        {//need to add query from kernel
            IMetadata::IEntry entryA(MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE);
            entryA.push_back(3000, Type2Type<MINT64>());
            entryA.push_back(3000000000, Type2Type<MINT64>());
            metadataA.update(MTK_SENSOR_INFO_EXPOSURE_TIME_RANGE, entryA);
        }

        {//need to add query from kernel
            IMetadata::IEntry entryA(MTK_SENSOR_INFO_MAX_FRAME_DURATION);
            entryA.push_back(50000000000, Type2Type<MINT64>());
            metadataA.update(MTK_SENSOR_INFO_MAX_FRAME_DURATION, entryA);
        }

        {//need to add query from kernel
            IMetadata::IEntry entryA(MTK_SENSOR_INFO_PHYSICAL_SIZE);
            entryA.push_back(5.82, Type2Type<MFLOAT>());
            entryA.push_back(4.76, Type2Type<MFLOAT>());
            metadataA.update(MTK_SENSOR_INFO_PHYSICAL_SIZE, entryA);
        }

        {//need to add query from kernel
            IMetadata::IEntry entryA(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE);
            MSize Size1(4000,3000);
            entryA.push_back(Size1, Type2Type<MSize>());
            metadataA.update(MTK_SENSOR_INFO_PIXEL_ARRAY_SIZE, entryA);
        }

        {//need to add query from kernel
            IMetadata::IEntry entryA(MTK_SENSOR_INFO_WHITE_LEVEL);
            switch (sensorStaticInfo[idx].rawSensorBit) {
                case RAW_SENSOR_8BIT:
                    s32Para = 256;
                    break;
                case RAW_SENSOR_10BIT:
                    s32Para = 1024;
                    break;
                case RAW_SENSOR_12BIT:
                    s32Para = 4096;
                    break;
                case RAW_SENSOR_14BIT:
                    s32Para = 16384;
                    break;
                default:
                    s32Para = 256;
                    break;
            }
            entryA.push_back(s32Para, Type2Type<MINT32>());
            metadataA.update(MTK_SENSOR_INFO_WHITE_LEVEL, entryA);
        }

        {
            IMetadata::IEntry entryA(MTK_SENSOR_INFO_ORIENTATION);
            entryA.push_back((MINT32)sensorStaticInfo[idx].orientationAngle, Type2Type<MINT32>());
            metadataA.update(MTK_SENSOR_INFO_ORIENTATION, entryA);
        }

        {
            IMetadata::IEntry entryA(MTK_SENSOR_INFO_PACKAGE);
                {
                    IMetadata metadataB;
                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                        entryB.push_back((MINT32)SENSOR_SCENARIO_ID_NORMAL_PREVIEW,Type2Type<MINT32>());
                        metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                        entryB.push_back((MINT32)sensorStaticInfo[idx].previewFrameRate,Type2Type<MINT32>());
                        metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                        MSize size1(sensorStaticInfo[idx].previewWidth,sensorStaticInfo[idx].previewHeight);
                        entryB.push_back(size1,Type2Type<MSize>());
                        metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                        MRect region1(MPoint(0,0), MSize(sensorStaticInfo[idx].previewWidth,sensorStaticInfo[idx].previewHeight));
                        entryB.push_back(region1,Type2Type<MRect>());
                        metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
                    }

                    entryA.push_back(metadataB,Type2Type<IMetadata>());
                }

                {
                    IMetadata metadataB;
                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                        entryB.push_back((MINT32)SENSOR_SCENARIO_ID_NORMAL_CAPTURE,Type2Type<MINT32>());
                        metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                        entryB.push_back((MINT32)sensorStaticInfo[idx].captureFrameRate,Type2Type<MINT32>());
                        metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                        MSize size1(sensorStaticInfo[idx].captureWidth,sensorStaticInfo[idx].captureHeight);
                        entryB.push_back(size1,Type2Type<MSize>());
                        metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                        MRect region1(MPoint(0,0), MSize(sensorStaticInfo[idx].captureWidth,sensorStaticInfo[idx].captureHeight));
                        entryB.push_back(region1,Type2Type<MRect>());
                        metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
                    }

                    entryA.push_back(metadataB,Type2Type<IMetadata>());
                }

                {
                    IMetadata metadataB;
                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                        entryB.push_back((MINT32)SENSOR_SCENARIO_ID_NORMAL_VIDEO,Type2Type<MINT32>());
                        metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                        entryB.push_back((MINT32)sensorStaticInfo[idx].videoFrameRate,Type2Type<MINT32>());
                        metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                        MSize size1(sensorStaticInfo[idx].videoWidth,sensorStaticInfo[idx].videoHeight);
                        entryB.push_back(size1,Type2Type<MSize>());
                        metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                        MRect region1(MPoint(0,0), MSize(sensorStaticInfo[idx].videoWidth,sensorStaticInfo[idx].videoHeight));
                        entryB.push_back(region1,Type2Type<MRect>());
                        metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
                    }

                    entryA.push_back(metadataB,Type2Type<IMetadata>());
                }

                {
                    IMetadata metadataB;
                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                        entryB.push_back((MINT32)SENSOR_SCENARIO_ID_SLIM_VIDEO1,Type2Type<MINT32>());
                        metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                        entryB.push_back((MINT32)sensorStaticInfo[idx].video1FrameRate,Type2Type<MINT32>());
                        metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                        MSize size1(sensorStaticInfo[idx].video1Width,sensorStaticInfo[idx].video1Height);
                        entryB.push_back(size1,Type2Type<MSize>());
                        metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                        MRect region1(MPoint(0,0), MSize(sensorStaticInfo[idx].video1Width,sensorStaticInfo[idx].video1Height));
                        entryB.push_back(region1,Type2Type<MRect>());
                        metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
                    }

                    entryA.push_back(metadataB,Type2Type<IMetadata>());
                }

                {
                    IMetadata metadataB;
                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_SCENARIO_ID);
                        entryB.push_back((MINT32)SENSOR_SCENARIO_ID_SLIM_VIDEO2,Type2Type<MINT32>());
                        metadataB.update(MTK_SENSOR_INFO_SCENARIO_ID, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_FRAME_RATE);
                        entryB.push_back((MINT32)sensorStaticInfo[idx].video2FrameRate,Type2Type<MINT32>());
                        metadataB.update(MTK_SENSOR_INFO_FRAME_RATE, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE);
                        MSize size1(sensorStaticInfo[idx].video2Width,sensorStaticInfo[idx].video2Height);
                        entryB.push_back(size1,Type2Type<MSize>());
                        metadataB.update(MTK_SENSOR_INFO_REAL_OUTPUT_SIZE, entryB);
                    }

                    {
                        IMetadata::IEntry entryB(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY);
                        MRect region1(MPoint(0,0), MSize(sensorStaticInfo[idx].video2Width,sensorStaticInfo[idx].video2Height));
                        entryB.push_back(region1,Type2Type<MRect>());
                        metadataB.update(MTK_SENSOR_INFO_OUTPUT_REGION_ON_ACTIVE_ARRAY, entryB);
                    }

                    entryA.push_back(metadataB,Type2Type<IMetadata>());
                }
            metadataA.update(MTK_SENSOR_INFO_PACKAGE, entryA);
        }
        metadataA.sort();

    }

    MY_LOGD("impBuildSensorInfo end!\n");


}



