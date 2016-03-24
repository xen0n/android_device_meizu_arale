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

#define LOG_TAG "testAdapter"
//
#include <stdio.h>
#include <stdlib.h>
//
#include <camera/MtkCamera.h>
#include <CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <BaseCamAdapter.h>
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include "Log.h"
//
#include <utils/threads.h>
#include <semaphore.h>
#include <sys/prctl.h>
//
//
using namespace NSCam;
//
//
sp<ICamAdapter> createMtkDefaultCamAdapter(String8 const& rName, int32_t const i4OpenId, sp<IParamsManager> pParamsMgr);
//
//
pthread_t     g_TestThread;
sem_t         g_semTestThreadStart;
sem_t         g_semTestThreadEnd;
//
//
/*******************************************************************************
*
********************************************************************************/
void* TestThreadFunc(void *arg)
{
    ::prctl(PR_SET_NAME,"TestThread", 0, 0, 0);
    MUINT32 sensorId = reinterpret_cast<MUINT32>(arg);
    //
    uint32_t    sensorArray[1];
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    IHalSensor* pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, sensorId);
    if(pSensorHalObj == NULL)
    {
       MY_LOGE("mpSensorHalObj is NULL");
    }
    //
    sensorArray[0] = sensorId;
    pSensorHalObj->powerOn(LOG_TAG, 1, &sensorArray[0]);
    //
    IHal3A* pHal3a = IHal3A::createInstance(IHal3A::E_Camera_1, sensorId, LOG_TAG);

    sp<ICamAdapter> pAdapter = createMtkDefaultCamAdapter(String8("Default"), sensorId, NULL);
    //
    MY_LOGD("init   (%d)", sensorId);
    pAdapter->init();
    MY_LOGD("startPreview   (%d)", sensorId);
    pAdapter->startPreview();
    //
    ::sem_wait(&g_semTestThreadStart);
    //
    usleep(10000000);
    //
    MY_LOGD("stopPreview   (%d)", sensorId);
    pAdapter->stopPreview();
    MY_LOGD("uninit   (%d)", sensorId);
    pAdapter->uninit();
    //
    ::sem_post(&g_semTestThreadEnd);
    //
    pHal3a->destroyInstance(LOG_TAG);
    //
    if(pSensorHalObj)
    {
        pSensorHalObj->powerOff(LOG_TAG, 1, &sensorArray[0]);
        pSensorHalObj->destroyInstance(LOG_TAG);
        pSensorHalObj = NULL;
    }
    return NULL;
}


/*******************************************************************************
*  Main Function
********************************************************************************/
int test_normal_preview(int argc, char** argv)
{
    int ret = 0;
    if( argc < 2 )
    {
        printf("Usage: camtest 0(normal preview) <sensorId>\n");
        printf("sensorId: 0(MAIN), 1(SUB) \n");
        return -1;
    }
    //
    MY_LOGD("Sensor:list get");
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MY_LOGD("Sensor:search");
    pHalSensorList->searchSensors();
    //
    MUINT32 sensorId = atoi(argv[1]);
    //
    sp<ICamAdapter> pAdapter = createMtkDefaultCamAdapter(String8("Default"), sensorId, NULL);
    //
    MY_LOGD("init   (%d)", sensorId);
    pAdapter->init();
    MY_LOGD("startPreview   (%d)", sensorId);
    pAdapter->startPreview();
    //
    usleep(10000000);
    //
    MY_LOGD("stopPreview   (%d)", sensorId);
    pAdapter->stopPreview();
    MY_LOGD("uninit   (%d)", sensorId);
    pAdapter->uninit();
    //
    return ret;
}


int test_pip_preview(int argc, char** argv)
{
    int ret = 0;
    if( argc < 2 )
    {
        printf("Usage: camtest 1(PIP preview) <open_id>\n");
        printf("open_id: 0(main->sub), 1(sub->main) \n");
        return -1;
    }
    //
    MY_LOGD("Sensor:list get");
    uint32_t    sensorArray[1];
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MY_LOGD("Sensor:search");
    pHalSensorList->searchSensors();
    //
    MUINT32 sensorId = atoi(argv[1]);
    //
    IHalSensor* pSensorHalObj = pHalSensorList->createSensor(LOG_TAG, sensorId);
    if(pSensorHalObj == NULL)
    {
       MY_LOGE("mpSensorHalObj is NULL");
    }
    //
    sensorArray[0] = sensorId;
    pSensorHalObj->powerOn(LOG_TAG, 1, &sensorArray[0]);
    //
    IHal3A* pHal3a = IHal3A::createInstance(IHal3A::E_Camera_1, sensorId, LOG_TAG);
    //
    sp<ICamAdapter> pAdapter = createMtkDefaultCamAdapter(String8("Default"), sensorId, NULL);
    //
    sem_init(&g_semTestThreadStart, 0, 0);
    sem_init(&g_semTestThreadEnd, 0, 0);
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_RR, 90};
    pthread_create(&g_TestThread, &attr, TestThreadFunc, (sensorId == 0) ? (void*)1 : (void*)0);
    //
    MY_LOGD("init   (%d)", sensorId);
    pAdapter->init();
    MY_LOGD("startPreview   (%d)", sensorId);
    pAdapter->startPreview();
    //
    ::sem_post(&g_semTestThreadStart);
    //
    usleep(10000000);
    //
    ::sem_wait(&g_semTestThreadEnd);
    //
    MY_LOGD("stopPreview   (%d)", sensorId);
    pAdapter->stopPreview();
    MY_LOGD("uninit   (%d)", sensorId);
    pAdapter->uninit();
    //
    pHal3a->destroyInstance(LOG_TAG);
    //
    if(pSensorHalObj)
    {
        pSensorHalObj->powerOff(LOG_TAG, 1, &sensorArray[0]);
        pSensorHalObj->destroyInstance(LOG_TAG);
        pSensorHalObj = NULL;
    }
    //
    return ret;
}
