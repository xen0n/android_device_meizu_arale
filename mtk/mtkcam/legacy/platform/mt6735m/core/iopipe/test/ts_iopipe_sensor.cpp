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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief
#include "ts_iopipe.h"

/*******************************************************************************
*  Sensor init
********************************************************************************/
int
Ts_UT::
main_SetSensor_init()
{
    halSensorIFParam_t halSensorIFParam[2];
    halSensorRawImageInfo_t sensorFormatInfo;
    MINT32 ret;

    printf("[SetSensor_init] E\n");

#if 0
    MUINT32 tg_grab_w = 1274;
    MUINT32 tg_grab_h = 948;
    MUINT32 max_prv_w = tg_grab_w;
    MUINT32 max_prv_h = tg_grab_h;
#else
    MUINT32 tg_grab_w = 0;
    MUINT32 tg_grab_h = 0;
    MUINT32 max_prv_w = 0;
    MUINT32 max_prv_h = 0;
#endif

    //====== Sensor_Hal Setting ======

    //search sensor
    printf("[SetSensor_init] SensorHal::createInstance\n");
    mpSensorHal = SensorHal::createInstance();

    printf("[SetSensor] mpSensorHal->searchSensor\n");
    mpSensorHal->searchSensor();
    ret = mpSensorHal->sendCommand(sensorDevId,
                                  SENSOR_CMD_SET_SENSOR_DEV,
                                  0,
                                  0,
                                  0);
    if(ret < 0)
    {
        printf("[SetSensor_init] SENSOR_CMD_SET_SENSOR_DEV fail\n");
        MY_LOGE("SENSOR_CMD_SET_SENSOR_DEV fail\n");
        return ret;
    }

    // init
    printf("[SetSensor_init] mpSensorHal->init\n");
    ret = mpSensorHal->init();
    if(ret < 0)
    {
        printf("[SetSensor_init] mpSensorHal->init fail\n");
        MY_LOGE("pSensorHal->init fail\n");
        return ret;
    }

    //====== Get Sensor Size ======
    printf("[SetSensor_init] mpSensorHal SENSOR_CMD_GET_SENSOR_PRV_RANGE\n");
    ret = mpSensorHal->sendCommand(sensorDevId,
                                  SENSOR_CMD_GET_SENSOR_PRV_RANGE,
                                  (MINT32)&u4SensorHalfWidth,
                                  (MINT32)&u4SensorHalfHeight,
                                  0);
    if(ret < 0)
    {
        printf("[SetSensor_init] SENSOR_CMD_GET_SENSOR_PRV_RANGE fail\n");
        MY_LOGE("SENSOR_CMD_GET_SENSOR_PRV_RANGE fail\n");
        return ret;
    }

    printf("[SetSensor_init] mpSensorHal SENSOR_CMD_GET_SENSOR_FULL_RANGE\n");
    ret = mpSensorHal->sendCommand(sensorDevId,
                                  SENSOR_CMD_GET_SENSOR_FULL_RANGE,
                                  (MINT32)&u4SensorFullWidth,
                                  (MINT32)&u4SensorFullHeight,
                                  0);
    if(ret < 0)
    {
        printf("[SetSensor_init] SENSOR_CMD_GET_SENSOR_FULL_RANGE\n");
        MY_LOGE("SENSOR_CMD_GET_SENSOR_FULL_RANGE fail\n");
        return ret;
    }

    printf("[SetSensor_init] Sensor resolution, Prv(%d,%d),Full(%d,%d)\n",u4SensorHalfWidth,
                                                                    u4SensorHalfHeight,
                                                                    u4SensorFullWidth,
                                                                    u4SensorFullHeight);

    MY_LOGD("Sensor resolution, Prv(%d,%d),Full(%d,%d)\n",u4SensorHalfWidth,
                                                            u4SensorHalfHeight,
                                                            u4SensorFullWidth,
                                                            u4SensorFullHeight);

    printf("[SetSensor_init] mpSensorHal SENSOR_CMD_GET_RAW_INFO\n");
    ret = mpSensorHal->sendCommand(sensorDevId,
                                  SENSOR_CMD_GET_RAW_INFO,
                                  (MINT32)&sensorFormatInfo,
                                  1,
                                  0);
    if(ret < 0)
    {
        printf("[SetSensor_init] SENSOR_CMD_GET_RAW_INFO  fail(0x%x)\n",ret);
        MY_LOGE("SENSOR_CMD_GET_RAW_INFO fail(0x%x)",ret);
        return ret;
    }

    if( mode == 0 )
    {
        u4SensorWidth = u4SensorHalfWidth;
        u4SensorHeight = u4SensorHalfHeight;
    }else{
        u4SensorWidth = u4SensorFullWidth;
        u4SensorHeight = u4SensorFullHeight;
    }

    printf("[SetSensor_init] X\n");
    return 0;
}

/*******************************************************************************
*  Sensor init /floria
********************************************************************************/
int
Ts_UT::
main_SetSensor_uninit()
{
    printf("[SetSensor_uninit] X\n");
    if(mpSensorHal)
    {
        mpSensorHal->uninit();
        mpSensorHal->destroyInstance();
        mpSensorHal = NULL;
    }
    printf("[SetSensor_uninit] E\n");
    return 0;
}
