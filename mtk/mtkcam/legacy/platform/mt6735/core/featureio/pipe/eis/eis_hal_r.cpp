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
*      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file eis_hal.cpp
*
* EIS Hal Source File
*
*/

#include <stdlib.h>
#include <stdio.h>
#include <utils/threads.h>
#include <cutils/log.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <linux/cache.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <queue>
#define MTK_LOG_ENABLE 1
using namespace std;

#include "eis_drv.h"
#include "mtkcam/drv/imem_drv.h"

#include <android/sensor.h>
#include <mtkcam/utils/SensorListener.h>
#include <mtkcam/Log.h>
using namespace android;

#include "mtkcam/v1/config/PriorityDefs.h"
#include "mtkcam/hal/IHalSensor.h"

using namespace NSCam;

#include "eis_hal_imp_r.h"
#include "camera_custom_eis.h"

/*******************************************************************************
*
********************************************************************************/
#define EIS_HAL_DEBUG

#ifdef EIS_HAL_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#define LOG_TAG "EisHal_R"
#define EIS_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define EIS_INF(fmt, arg...)    CAM_LOGI("[%s]" fmt, __func__, ##arg)
#define EIS_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define EIS_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else
#define EIS_LOG(a,...)
#define EIS_INF(a,...)
#define EIS_WRN(a,...)
#define EIS_ERR(a,...)
#endif

#define EIS_HAL_NAME "EisHal_R"
#define intPartShift 8
#define floatPartShift (31 - intPartShift)
#define DEBUG_DUMP_FRAMW_NUM 10

/*******************************************************************************
*
********************************************************************************/
static MINT32 g_debugDump = 0;
static MFLOAT gAccInfo[3] = {0};
static MFLOAT gGyroInfo[3] = {0};

/*******************************************************************************
*
********************************************************************************/
void mySensorListener_R(ASensorEvent event)
{
    static MUINT32 accCnt = 1;
    static MUINT32 gyroCnt = 1;

    switch(event.type)
    {
        case ASENSOR_TYPE_ACCELEROMETER:
        {
            if(g_debugDump >= 2)
            {
                EIS_LOG("[%u] Acc(%f,%f,%f,%lld)",
                        accCnt++,
                        event.acceleration.x,
                        event.acceleration.y,
                        event.acceleration.z,
                        event.timestamp);
            }

            gAccInfo[0] = event.acceleration.x;
            gAccInfo[1] = event.acceleration.y;
            gAccInfo[2] = event.acceleration.z;

            break;
        }
        case ASENSOR_TYPE_MAGNETIC_FIELD:
        {
            if(g_debugDump >= 2)
            {
                EIS_LOG("Mag");
            }
            break;
        }
        case ASENSOR_TYPE_GYROSCOPE:
        {
            if(g_debugDump >= 2)
            {
                EIS_LOG("[%u] Gyro(%f,%f,%f,%lld)",
                        gyroCnt++,
                        event.vector.x,
                        event.vector.y,
                        event.vector.z,
                        event.timestamp);
            }

            gGyroInfo[0] = event.vector.x;
            gGyroInfo[1] = event.vector.y;
            gGyroInfo[2] = event.vector.z;

            break;
        }
        case ASENSOR_TYPE_LIGHT:
        {
            if(g_debugDump >= 2)
            {
                EIS_LOG("Light");
            }
            break;
        }
        case ASENSOR_TYPE_PROXIMITY:
        {
            if(g_debugDump >= 2)
            {
                EIS_LOG("Proxi");
            }
            break;
        }
        default:
        {
            EIS_WRN("unknown type(%d)",event.type);
            break;
        }
    }

}


/*******************************************************************************
*
********************************************************************************/
EisHal_R *EisHal_R::CreateInstance(char const *userName,const MUINT32 &aSensorIdx)
{
    EIS_LOG("%s",userName);
    return EisHalImp_R::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
EisHal_R *EisHalImp_R::GetInstance(const MUINT32 &aSensorIdx)
{
    EIS_LOG("sensorIdx(%u)",aSensorIdx);

    switch(aSensorIdx)
    {
        case 0 : return EisHalObj_R<0>::GetInstance();
        case 1 : return EisHalObj_R<1>::GetInstance();
        case 2 : return EisHalObj_R<2>::GetInstance();
        case 3 : return EisHalObj_R<3>::GetInstance();
        default :
            EIS_WRN("Current limit is 4 sensors, use 0");
            return EisHalObj_R<0>::GetInstance();
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::DestroyInstance(char const *userName)
{
    EIS_LOG("%s",userName);
}

/*******************************************************************************
*
********************************************************************************/
EisHalImp_R::EisHalImp_R(const MUINT32 &aSensorIdx) : EisHal_R()
{
    mUsers = 0;

    //> EIS driver object
    m_pEisDrv = NULL;

    //> member variable
    mEisInput_W = 0;
    mEisInput_H = 0;
    mP1Target_W = 0;
    mP1Target_H = 0;
    mSrzOutW = 0;
    mSrzOutH = 0;
    mDoEisCount = 0;    //Vent@20140427: Add for EIS GMV Sync Check.
    mCmvX_Int = 0;
    mCmvX_Flt = 0;
    mCmvY_Int = 0;
    mCmvY_Flt = 0;
    mGMV_X = 0;
    mGMV_Y = 0;
    mFrameCnt = 0;
    mEisPass1Only = 1;
    mIsEisConfig = 0;
    mIsEisPlusConfig = 0;
    mSensorIdx = aSensorIdx;
    mEisSupport = MTRUE;
    mMemAlignment = 0;
    mEisPlusCropRatio = 20;
    mEisP2UserCnt = 0;
    mGyroEnable = MFALSE;
    mAccEnable  = MFALSE;

#if EIS_ALGO_READY

    m_pEisAlg = NULL;
    m_pEisPlusAlg = NULL;

#endif

    //> IMEM
    m_pIMemDrv = NULL;

    mEisDbgBuf.memID = -5;
    mEisDbgBuf.virtAddr = mEisDbgBuf.phyAddr = mEisDbgBuf.size = 0;

    mEisPlusWorkBuf.memID = -5;
    mEisPlusWorkBuf.virtAddr = mEisPlusWorkBuf.phyAddr = mEisPlusWorkBuf.size = 0;

    //> FE
    mFeoStatData.feX = NULL;
    mFeoStatData.feY = NULL;
    mFeoStatData.feRes = NULL;
    mFeoStatData.feDes = NULL;
    mFeoStatData.feValid = NULL;

    // sensor listener
    mpSensorListener = NULL;

    // sensor
    m_pHalSensorList = NULL;
    m_pHalSensor = NULL;

    // eis result data
    while(!mEis2EisPlusGmvX.empty())
    {
        mEis2EisPlusGmvX.pop();
    }

    while(!mEis2EisPlusGmvY.empty())
    {
        mEis2EisPlusGmvY.pop();
    }

    while(!mEis2EisPlusConfX.empty())
    {
        mEis2EisPlusConfX.pop();
    }

    while(!mEis2EisPlusConfY.empty())
    {
        mEis2EisPlusConfY.pop();
    }

    while(!mEisResultForP2.empty())
    {
        mEisResultForP2.pop();
    }

#if EIS_WORK_AROUND

    mTgRrzRatio = 0;

#endif

}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp_R::Init()
{
    //====== Check Reference Count ======

    Mutex::Autolock lock(mLock);

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        EIS_LOG("snesorIdx(%u) has %d users",mSensorIdx,mUsers);
        return EIS_RETURN_NO_ERROR;
    }

    MINT32 err = EIS_RETURN_NO_ERROR;

    //====== Dynamic Debug ======

#if (EIS_DEBUG_FLAG)

    EIS_INF("EIS_DEBUG_FLAG on");
    g_debugDump = 1;

#else

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.eis.dump", value, "0");
    g_debugDump = atoi(value);

#endif

    EIS_LOG("mSensorIdx(%u) init", mSensorIdx);

    //====== Create Sensor Object ======

    m_pHalSensorList = IHalSensorList::get();
    if(m_pHalSensorList == NULL)
    {
        EIS_ERR("IHalSensorList::get fail");
        goto create_fail_exit;
    }

    if(EIS_RETURN_NO_ERROR != GetSensorInfo())
    {
        EIS_ERR("GetSensorInfo fail");
        goto create_fail_exit;
    }

    //====== Create EIS Driver ======

    m_pEisDrv = EisDrv_R::CreateInstance(mSensorIdx);

    if(m_pEisDrv == NULL)
    {
        EIS_ERR("EisDrv::createInstance fail");
        goto create_fail_exit;
    }

    err = m_pEisDrv->Init();
    if(err != EIS_RETURN_NO_ERROR)
    {
        EIS_ERR("EisDrv::Init fail");
        goto create_fail_exit;
    }

    //====== Create EIS Algorithm Object ======

#if EIS_ALGO_READY

    EIS_LOG("TG(%d)",mSensorDynamicInfo.TgInfo);

    if(mSensorDynamicInfo.TgInfo == CAM_TG_1)
    {
        m_pEisAlg = MTKEis::createInstance();

        if(m_pEisAlg == NULL)
        {
            EIS_ERR("MTKEis::createInstance fail");
            goto create_fail_exit;
        }
    }

    if(mSensorDynamicInfo.TgInfo == CAM_TG_1 && m_pEisAlg == NULL)
    {
        EIS_ERR("m_pEisAlg is NULL");
        goto create_fail_exit;
    }

    m_pEisPlusAlg = MTKEisPlus::createInstance();

    if(m_pEisPlusAlg == NULL)
    {
        EIS_ERR("MTKEisPlus::createInstance fail");
        goto create_fail_exit;
    }

    //> get GPU info only

    EIS_PLUS_GET_PROC_INFO_STRUCT eisPlusGetProcData;

    err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_GET_PROC_INFO, NULL, &eisPlusGetProcData);
    if(err != S_EIS_PLUS_OK)
    {
        EIS_ERR("get GPU info fail(0x%x)",err);
        goto create_fail_exit;
    }

    // only Grid_W and Grid_H are valid here

    EIS_LOG("Grid:(W/H)=(%u/%u)",eisPlusGetProcData.Grid_W,eisPlusGetProcData.Grid_H);

    mGpuGridW = eisPlusGetProcData.Grid_W;
    mGpuGridH = eisPlusGetProcData.Grid_H;

    //> IMem

    m_pIMemDrv = IMemDrv::createInstance();

    if(m_pIMemDrv == NULL)
    {
        EIS_ERR("Null IMemDrv Obj");
        goto create_fail_exit;
    }

    if(MTRUE != m_pIMemDrv->init())
    {
        EIS_ERR("m_pIMemDrv->init fail");
        goto create_fail_exit;
    }

    if(g_debugDump >= 2)
    {
        //====== EIS Debug Buffer ======

        MUINT32 eisMemSize = 0;

        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_DEBUG_INFO,NULL,&eisMemSize);
        if(err != S_EIS_OK || eisMemSize == 0)
        {
            EIS_ERR("EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO) fail(0x%x)",err);
        }
        else
        {
            CreateMemBuf(eisMemSize,1,&mEisDbgBuf);
            if(mEisDbgBuf.virtAddr == 0 && mEisDbgBuf.phyAddr == 0)
            {
                EIS_ERR("mEisDbgBuf create IMem fail");
                return EIS_RETURN_MEMORY_ERROR;
            }

            EIS_LOG("mEisDbgBuf : memID(%d),size(%u),virAdd(0x%x),phyAddr(0x%x)",mEisDbgBuf.memID,
                                                                                 mEisDbgBuf.size,
                                                                                 mEisDbgBuf.virtAddr,
                                                                                 mEisDbgBuf.phyAddr);
        }
    }

    //====== Create Sensor Listener Object ======

    mpSensorListener = SensorListener::createInstance();

    if(MTRUE != mpSensorListener->setListener(mySensorListener_R))
    {
        EIS_ERR("setListener fail");
    }
    else
    {
        EIS_LOG("setListener success");
    }

    mAccEnable  = mpSensorListener->enableSensor(SensorListener::SensorType_Acc,33);
    mGyroEnable = mpSensorListener->enableSensor(SensorListener::SensorType_Gyro,33);
    EIS_LOG("EN:(Acc,Gyro)=(%d,%d)",mAccEnable,mGyroEnable);

#endif

    //====== Create FEO Memory ======

    mFeoStatData.feX     = new MUINT16[MAX_FEO_SIZE];
    mFeoStatData.feY     = new MUINT16[MAX_FEO_SIZE];
    mFeoStatData.feRes   = new MUINT16[MAX_FEO_SIZE];
    mFeoStatData.feDes   = new MUINT16[MAX_FEO_SIZE*32];
    mFeoStatData.feValid = new MUINT8[MAX_FEO_SIZE];

    memset(mFeoStatData.feX,0,sizeof(MUINT16)*MAX_FEO_SIZE);
    memset(mFeoStatData.feY,0,sizeof(MUINT16)*MAX_FEO_SIZE);
    memset(mFeoStatData.feRes,0,sizeof(MUINT16)*MAX_FEO_SIZE);
    memset(mFeoStatData.feDes,0,sizeof(MUINT16)*MAX_FEO_SIZE*32);
    memset(mFeoStatData.feValid,0,sizeof(MUINT8)*MAX_FEO_SIZE);

    //====== Get EIS Plus Crop Ratio ======

    EIS_PLUS_Customize_Para_t customSetting;

    get_EIS_PLUS_CustomizeData(&customSetting);

    mEisPlusCropRatio = 100 + customSetting.crop_ratio;
    EIS_LOG("mEisPlusCropRatio(%u)",mEisPlusCropRatio);

    //====== Increase User Count ======

    android_atomic_inc(&mUsers);

    EIS_LOG("-");
    return EIS_RETURN_NO_ERROR;

create_fail_exit:

    if(m_pEisDrv != NULL)
    {
        m_pEisDrv->Uninit();
        m_pEisDrv->DestroyInstance();
        m_pEisDrv = NULL;
    }

    if(m_pHalSensorList != NULL)
    {
        m_pHalSensorList = NULL;
    }

#if EIS_ALGO_READY

    if(m_pEisAlg != NULL)
    {
        m_pEisAlg->EisReset();
        m_pEisAlg->destroyInstance();
        m_pEisAlg = NULL;
    }

    if(m_pEisPlusAlg != NULL)
    {
        m_pEisPlusAlg->EisPlusReset();
        m_pEisPlusAlg->destroyInstance(m_pEisPlusAlg);
        m_pEisPlusAlg = NULL;
    }

    if(m_pIMemDrv != NULL)
    {
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }

#endif

    EIS_LOG("-");
    return EIS_RETURN_NULL_OBJ;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp_R::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        EIS_LOG("mSensorIdx(%u) has 0 user",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)    // there is no user
    {
        MINT32 err = EIS_RETURN_NO_ERROR;

        EIS_LOG("mSensorIdx(%u) uninit", mSensorIdx);

        //====== Release EIS Driver Object ======

        //err = m_pEisDrv->EnableEis(MFALSE);   // ISP will reset TOP EN bit
        //if(err != EIS_RETURN_NO_ERROR)
        //{
        //    EIS_ERR("Disable EIS fail(0x%x)",err);
        //}

        if(m_pEisDrv != NULL)
        {
            EIS_LOG("m_pEisDrv uninit");
            m_pEisDrv->SetEisoThreadState(EIS_SW_STATE_UNINIT_CHECK);
            m_pEisDrv->Uninit();
            m_pEisDrv->DestroyInstance();
            m_pEisDrv = NULL;
        }

        //======  Release EIS Algo Object ======

#if EIS_ALGO_READY

        if(g_debugDump >= 2)
        {
            if(mIsEisPlusConfig == 1)
            {
                err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_SAVE_LOG,NULL,NULL);
                if(err != S_EIS_PLUS_OK)
                {
                    EIS_ERR("EisPlusFeatureCtrl(EIS_PLUS_FEATURE_SAVE_LOG) fail(0x%x)",err);
                }
            }

            if(mSensorDynamicInfo.TgInfo == CAM_TG_1)
            {
                err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SAVE_LOG, NULL, NULL);
                if(err != S_EIS_OK)
                {
                    EIS_ERR("EisFeatureCtrl(EIS_FEATURE_SAVE_LOG) fail(0x%x)",err);
                }
            }
        }

        EIS_LOG("TG(%d)",mSensorDynamicInfo.TgInfo);

        if(m_pEisAlg != NULL && mSensorDynamicInfo.TgInfo == CAM_TG_1)
        {
            EIS_LOG("m_pEisAlg uninit");
            m_pEisAlg->EisReset();
            m_pEisAlg->destroyInstance();
            m_pEisAlg = NULL;
        }

        if(m_pEisPlusAlg != NULL)
        {
            EIS_LOG("m_pEisPlusAlg uninit");
            m_pEisPlusAlg->EisPlusReset();
            m_pEisPlusAlg->destroyInstance(m_pEisPlusAlg);
            m_pEisPlusAlg = NULL;
        }

        // Next-Gen EIS
        if(mpSensorListener != NULL)
        {
            EIS_LOG("mpSensorListener uninit");
            mpSensorListener->disableSensor(SensorListener::SensorType_Acc);
            mpSensorListener->disableSensor(SensorListener::SensorType_Gyro);
            mpSensorListener->destroyInstance();
            mpSensorListener = NULL;
        }

#endif

        //====== Destroy Sensor Object ======

        if(m_pHalSensorList != NULL)
        {
            m_pHalSensorList = NULL;
        }

        //======  Release Memory and IMem Object ======

        if(g_debugDump >= 2)
        {
            //>  free EIS debug buffer

            DestroyMemBuf(1,&mEisDbgBuf);

            mEisDbgBuf.memID = -5;
            mEisDbgBuf.virtAddr = mEisDbgBuf.phyAddr = mEisDbgBuf.size = 0;
        }

        //>  free EIS Plus working buffer

        DestroyMemBuf(1,&mEisPlusWorkBuf);

        mEisPlusWorkBuf.memID = -5;
        mEisPlusWorkBuf.virtAddr = mEisPlusWorkBuf.phyAddr = mEisPlusWorkBuf.size = 0;

        if(m_pIMemDrv != NULL)
        {
            EIS_LOG("m_pIMemDrv uninit");
            if(MTRUE != m_pIMemDrv->uninit())
            {
                EIS_ERR("m_pIMemDrv->uninit fail");
            }
            m_pIMemDrv->destroyInstance();
            m_pIMemDrv = NULL;
        }

        //======  Reset Member Variable ======

        mEisInput_W = 0;
        mEisInput_H = 0;
        mP1Target_W = 0;
        mP1Target_H = 0;
        mSrzOutW = 0;
        mSrzOutH = 0;
        mFrameCnt = 0; // first frmae
        mEisPass1Only = 1;
        mIsEisConfig = 0;
        mIsEisPlusConfig = 0;
        mCmvX_Int = 0;
        mDoEisCount = 0;    //Vent@20140427: Add for EIS GMV Sync Check.
        mCmvX_Flt = 0;
        mCmvY_Int = 0;
        mCmvY_Flt = 0;
        mGMV_X = 0;
        mGMV_Y = 0;
        mGpuGridW = 0;
        mGpuGridH = 0;
        mMemAlignment = 0;
        mEisPlusCropRatio = 20;
        mEisP2UserCnt = 0;
        mGyroEnable = MFALSE;
        mAccEnable  = MFALSE;

    #if EIS_WORK_AROUND

        mTgRrzRatio = 0;

    #endif

        //> FE

        delete [] mFeoStatData.feX;
        delete [] mFeoStatData.feY;
        delete [] mFeoStatData.feRes;
        delete [] mFeoStatData.feDes;
        delete [] mFeoStatData.feValid;

        mFeoStatData.feX = NULL;
        mFeoStatData.feY = NULL;
        mFeoStatData.feRes = NULL;
        mFeoStatData.feDes = NULL;
        mFeoStatData.feValid = NULL;

        while(!mEis2EisPlusGmvX.empty())
        {
            mEis2EisPlusGmvX.pop();
        }

        while(!mEis2EisPlusGmvY.empty())
        {
            mEis2EisPlusGmvY.pop();
        }

        while(!mEis2EisPlusConfX.empty())
        {
            mEis2EisPlusConfX.pop();
        }

        while(!mEis2EisPlusConfY.empty())
        {
            mEis2EisPlusConfY.pop();
        }

        while(!mEisResultForP2.empty())
        {
            mEisResultForP2.pop();
        }
    }
    else
    {
        EIS_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp_R::CreateMemBuf(MUINT32 &memSize,const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo)
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    MUINT32 alingSize = (memSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1);

    EIS_LOG("Cnt(%d),Size(%u),alingSize(%u)",bufCnt, memSize, alingSize);

    memSize = alingSize;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            bufInfo[i].size = alingSize;

            if(m_pIMemDrv->allocVirtBuf(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->allocVirtBuf() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }

            if(m_pIMemDrv->mapPhyAddr(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->mapPhyAddr() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        bufInfo->size = alingSize;

        if(m_pIMemDrv->allocVirtBuf(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->allocVirtBuf() error");
            err = EIS_RETURN_API_FAIL;
        }

        if(m_pIMemDrv->mapPhyAddr(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->mapPhyAddr() error");
            err = EIS_RETURN_API_FAIL;
        }
    }

    EIS_LOG("X");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 EisHalImp_R::DestroyMemBuf(const MUINT32 & bufCnt, IMEM_BUF_INFO *bufInfo)
{
    EIS_LOG("Cnt(%d)", bufCnt);

    MINT32 err = EIS_RETURN_NO_ERROR;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            if(0 == bufInfo[i].virtAddr)
            {
                EIS_LOG("Buffer doesn't exist, i(%d)",i);
                continue;
            }

            if(m_pIMemDrv->unmapPhyAddr(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->unmapPhyAddr() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }

            if (m_pIMemDrv->freeVirtBuf(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->freeVirtBuf() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        if(0 == bufInfo->virtAddr)
        {
            EIS_LOG("Buffer doesn't exist");
            return EIS_RETURN_NO_ERROR;
        }

        if(m_pIMemDrv->unmapPhyAddr(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->unmapPhyAddr() error");
            err = EIS_RETURN_API_FAIL;
        }

        if (m_pIMemDrv->freeVirtBuf(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->freeVirtBuf() error");
            err = EIS_RETURN_API_FAIL;
        }
    }

    EIS_LOG("X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp_R::GetSensorInfo()
{
    EIS_LOG("mSensorIdx(%u)",mSensorIdx);

    mSensorDev = m_pHalSensorList->querySensorDevIdx(mSensorIdx);
    m_pHalSensorList->querySensorStaticInfo(mSensorDev,&mSensorStaticInfo);

    m_pHalSensor = m_pHalSensorList->createSensor(EIS_HAL_NAME,1,&mSensorIdx);
    if(m_pHalSensor == NULL)
    {
        EIS_ERR("m_pHalSensorList->createSensor fail");
        return EIS_RETURN_API_FAIL;
    }

    if(MFALSE == m_pHalSensor->querySensorDynamicInfo(mSensorDev,&mSensorDynamicInfo))
    {
        EIS_ERR("querySensorDynamicInfo fail");
        return EIS_RETURN_API_FAIL;
    }

    m_pHalSensor->destroyInstance(EIS_HAL_NAME);
    m_pHalSensor = NULL;

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp_R::ConfigEis(const EIS_PASS_ENUM &aEisPass,const EIS_HAL_CONFIG_DATA &aEisConfig)
{
    mEisSupport = m_pEisDrv->GetEisSupportInfo(mSensorIdx);

    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    MINT32 err = EIS_RETURN_NO_ERROR;

#if EIS_ALGO_READY

    static EIS_SET_ENV_INFO_STRUCT eisAlgoInitData;

#endif

    if(aEisPass == EIS_PASS_1)
    {
        //====== Get Pass1 Memory Alignment ======

        mMemAlignment = aEisConfig.memAlignment;

        if(g_debugDump >= 1)
        {
            EIS_LOG("mMemAlignment(%u)",mMemAlignment);
        }

        //====== Check EIS Configure Scenario ======

        switch(aEisConfig.configSce)
        {
            case EIS_SCE_EIS :
            case EIS_SCE_MFBLL :
            case EIS_SCE_MT :
            case EIS_SCE_MAV :
                mEisPass1Only = 1;
            break;
            case EIS_SCE_EIS_PLUS:
                //mEisPass1Only = 0;    //k-two not support EIS2.0
                EIS_ERR("k-two is EIS_SCE_EIS only");
            break;
            default :
                EIS_ERR("wrong EIS config scenario(%d)",aEisConfig.configSce);
                return EIS_RETURN_INVALID_PARA;
        }

        //====== Check Sensor Type ======

        EIS_SENSOR_ENUM sensorType;

        switch(aEisConfig.sensorType)
        {
            case NSCam::SENSOR_TYPE_RAW:
                sensorType = EIS_RAW_SENSOR;
                break;
            case NSCam::SENSOR_TYPE_YUV:
                sensorType = EIS_YUV_SENSOR;
                break;
            default:
                EIS_ERR("not support sensor type(%u), use RAW setting",aEisConfig.sensorType);
                sensorType = EIS_RAW_SENSOR;
                break;
        }

#if EIS_ALGO_READY

        //====== EIS Algo Init ======

        if(g_debugDump >= 1)
        {
            EIS_LOG("mIsEisConfig(%u)",mIsEisConfig);
        }

        if(mIsEisConfig == 0)
        {
            EIS_LOG("Sce(%d)",aEisConfig.configSce);

            //> get EIS customize setting

            GetEisCustomize(&eisAlgoInitData.eis_tuning_data);

            eisAlgoInitData.Eis_Input_Path = EIS_PATH_RAW_DOMAIN;   // RAW domain

            //> init EIS algo

            err = m_pEisAlg->EisInit(&eisAlgoInitData);
            if(err != S_EIS_OK)
            {
                EIS_ERR("EisInit fail(0x%x)",err);
                return EIS_RETURN_API_FAIL;
            }

            if(g_debugDump >= 2)
            {
                EIS_SET_LOG_BUFFER_STRUCT eisAlgoLogInfo;

                eisAlgoLogInfo.Eis_Log_Buf_Addr = (MVOID *)mEisDbgBuf.virtAddr;
                eisAlgoLogInfo.Eis_Log_Buf_Size = mEisDbgBuf.size;

                err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO, &eisAlgoLogInfo, NULL);
                if(err != S_EIS_OK)
                {
                    EIS_ERR("EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO) fail(0x%x)",err);
                }
            }
        }

        if(g_debugDump >= 1)
        {
            EIS_LOG("EIS tuning_data");
            EIS_LOG("sensitivity(%d)",eisAlgoInitData.eis_tuning_data.sensitivity);
            EIS_LOG("filter_small_motion(%u)",eisAlgoInitData.eis_tuning_data.filter_small_motion);
            EIS_LOG("adv_shake_ext(%u)",eisAlgoInitData.eis_tuning_data.adv_shake_ext);
            EIS_LOG("stabilization_strength(%u)",eisAlgoInitData.eis_tuning_data.stabilization_strength);
            EIS_LOG("advtuning_data.new_tru_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.new_tru_th);
            EIS_LOG("advtuning_data.vot_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.vot_th);
            EIS_LOG("advtuning_data.votb_enlarge_size(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.votb_enlarge_size);
            EIS_LOG("advtuning_data.min_s_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.min_s_th);
            EIS_LOG("advtuning_data.vec_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.vec_th);
            EIS_LOG("advtuning_data.spr_offset(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.spr_offset);
            EIS_LOG("advtuning_data.spr_gain1(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.spr_gain1);
            EIS_LOG("advtuning_data.spr_gain2(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.spr_gain2);
            EIS_LOG("advtuning_data.vot_his_method(%d)",eisAlgoInitData.eis_tuning_data.advtuning_data.vot_his_method);
            EIS_LOG("advtuning_data.smooth_his_step(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.smooth_his_step);
            EIS_LOG("advtuning_data.eis_debug(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.eis_debug);

            for(MINT32 i = 0; i < 4; ++i)
            {
                EIS_LOG("gmv_pan_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.gmv_pan_array[i]);
                EIS_LOG("gmv_sm_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.gmv_sm_array[i]);
                EIS_LOG("cmv_pan_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.cmv_pan_array[i]);
                EIS_LOG("cmv_sm_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.cmv_sm_array[i]);
            }
        }
#endif

        //===== EIS HW Config ======

        err = m_pEisDrv->ConfigEisReg(aEisConfig.configSce,mSensorDynamicInfo.TgInfo);
        if(err != EIS_RETURN_NO_ERROR)
        {
            EIS_ERR("ConfigEisReg fail(0x%x)",err);
            return EIS_RETURN_API_FAIL;
        }

        //====== Enable EIS ======

#if EIS_WORK_AROUND

        mTgRrzRatio = m_pEisDrv->GetTgRrzRatio();
        EIS_LOG("mTgRrzRatio(%u)",mTgRrzRatio);

        if(mTgRrzRatio == 0)
        {
            EIS_ERR("mTgRrzRatio is 0",mTgRrzRatio);
            mTgRrzRatio = 1;
        }
        else if(mTgRrzRatio == 2)
        {
            mTgRrzRatio++;
        }

#endif

        //====== Enable EIS ======

        if(mIsEisConfig == 0)
        {
            err = m_pEisDrv->EnableEis(MTRUE);
            if(err != EIS_RETURN_NO_ERROR)
            {
                EIS_ERR("Enable EIS fail(0x%x)",err);
                return EIS_RETURN_API_FAIL;
            }
        }

        //====== Turn on Eis Configure One Time Flag ======

        if(mIsEisConfig == 0)
        {
            // set EISO thread state
            m_pEisDrv->SetEisoThreadState(EIS_SW_STATE_ALIVE);
            mIsEisConfig = 1;
        }
    }
    else if(aEisPass == EIS_PASS_2)
    {
        if(mEisPass1Only == 1)
        {
            EIS_LOG("Only use EIS pass1");
            return EIS_RETURN_NO_ERROR;
        }
        else
        {
#if EIS_ALGO_READY

            if(mIsEisPlusConfig == 0)
            {
                EIS_LOG("EIS Plus first config");

                EIS_PLUS_SET_ENV_INFO_STRUCT eisPlusAlgoInitData;
                EIS_PLUS_GET_PROC_INFO_STRUCT eisPlusGetProcData;
                EIS_PLUS_SET_WORKING_BUFFER_STRUCT eisPlusWorkBufData;

                //> prepare eisPlusAlgoInitData

                eisPlusAlgoInitData.wide_angle_lens = NULL;
                eisPlusAlgoInitData.debug = MFALSE;

                // get EIS Plus customized data
                GetEisPlusCustomize(&eisPlusAlgoInitData.eis_plus_tuning_data);

                if(g_debugDump >= 2)
                {
                    eisPlusAlgoInitData.debug = MTRUE;
                    EIS_LOG("eisPlus debug(%d)",eisPlusAlgoInitData.debug);
                }

                EIS_LOG("EIS Plus tuning_data");
                EIS_LOG("warping_mode(%d),effort(%d)",eisPlusAlgoInitData.eis_plus_tuning_data.warping_mode,eisPlusAlgoInitData.eis_plus_tuning_data.effort);
                EIS_LOG("search_range(%d,%d)",eisPlusAlgoInitData.eis_plus_tuning_data.search_range_x,eisPlusAlgoInitData.eis_plus_tuning_data.search_range_y);
                EIS_LOG("crop_ratio(%d),stabilization_strength(%f)",eisPlusAlgoInitData.eis_plus_tuning_data.crop_ratio,eisPlusAlgoInitData.eis_plus_tuning_data.stabilization_strength);

                //> Init Eis plus

                err = m_pEisPlusAlg->EisPlusInit(&eisPlusAlgoInitData);
                if(err != S_EIS_PLUS_OK)
                {
                    EIS_ERR("EisPlusInit fail(0x%x)",err);
                    return EIS_RETURN_API_FAIL;
                }

                //> Preapre EIS Plus Working Buffer

                err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_GET_PROC_INFO, NULL, &eisPlusGetProcData);
                if(err != S_EIS_PLUS_OK)
                {
                    EIS_ERR("EisPlus: EIS_PLUS_FEATURE_GET_PROC_INFO fail(0x%x)",err);
                    return EIS_RETURN_API_FAIL;
                }

                EIS_LOG("ext_mem_size(%u)",eisPlusGetProcData.ext_mem_size);

                CreateMemBuf(eisPlusGetProcData.ext_mem_size,1,&mEisPlusWorkBuf);
                if(mEisPlusWorkBuf.virtAddr == 0 && mEisPlusWorkBuf.phyAddr == 0)
                {
                    EIS_ERR("mEisPlusWorkBuf create IMem fail");
                    return EIS_RETURN_MEMORY_ERROR;
                }

                EIS_LOG("mEisPlusWorkBuf : memID(%d),size(%u),virAdd(0x%8x),phyAddr(0x%8x)",mEisPlusWorkBuf.memID,
                                                                                            mEisPlusWorkBuf.size,
                                                                                            mEisPlusWorkBuf.virtAddr,
                                                                                            mEisPlusWorkBuf.phyAddr);

                eisPlusWorkBufData.extMemSize = mEisPlusWorkBuf.size;
                eisPlusWorkBufData.extMemStartAddr = (MVOID *)mEisPlusWorkBuf.virtAddr;

                err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_SET_WORK_BUF_INFO, &eisPlusWorkBufData, NULL);
                if(err != S_EIS_PLUS_OK)
                {
                    EIS_ERR("EisPlus: EIS_PLUS_FEATURE_SET_WORK_BUF_INFO fail(0x%x)",err);
                    return EIS_RETURN_API_FAIL;
                }

                //> confige FEO

                err = m_pEisDrv->ConfigFeo();
                if(err != EIS_RETURN_NO_ERROR)
                {
                    EIS_ERR("ConfigFeo fail(0x%x)",err);
                    return EIS_RETURN_API_FAIL;
                }

                mIsEisPlusConfig = 1;
            }
#endif
        }
    }
    else
    {
        EIS_ERR("Wrong EIS config pass(%d)",aEisPass);
    }

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp_R::DoEis(const EIS_PASS_ENUM &aEisPass,EIS_HAL_CONFIG_DATA *apEisConfig,MINT64 aTimeStamp)
{
    MINT32 err = EIS_RETURN_NO_ERROR;

    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    if(g_debugDump >= 1)
    {
        EIS_LOG("aEisPass(%d),mEisPass1Only(%u)",aEisPass,mEisPass1Only);
    }

    if(aEisPass == EIS_PASS_1)
    {

        if(aTimeStamp <= 0)
        {
            m_pEisDrv->GetEisHwStatistic(NULL,0);
        }
        else
        {
    #if EIS_ALGO_READY

            //====== Setting EIS Algo Process Data ======

            EIS_RESULT_INFO_STRUCT   eisCMVResult;

            {
                //> EIS input image size
                Mutex::Autolock lock(mP1Lock);

                mEisInput_W = apEisConfig->p1ImgW;
                mEisInput_H = apEisConfig->p1ImgH;

                if(m_pEisDrv->Get2PixelMode())
                {
                    mEisInput_W >>= 1;
                }

                mEisInput_W -= 4;   //ryan wang request to -4
                mEisInput_H -= 4;   //ryan wang request to -4

                if(mEisPass1Only == 1)
                {
                    if(g_debugDump == 1)
                    {
                        EIS_LOG("EIS Pass1 Only");
                    }

                    mP1Target_W = (mEisInput_W / (EIS_FACTOR / 100.0));
                    mP1Target_H = (mEisInput_H / (EIS_FACTOR / 100.0));
                }
                else
                {
                    if(g_debugDump == 1)
                    {
                        EIS_LOG("mEisPlusCropRatio(%u)",mEisPlusCropRatio);
                    }

                    mP1Target_W = (mEisInput_W / (mEisPlusCropRatio / 100.0));
                    mP1Target_H = (mEisInput_H / (mEisPlusCropRatio / 100.0));
                }

                mEisAlgoProcData.eis_image_size_config.InputWidth   = mEisInput_W;
                mEisAlgoProcData.eis_image_size_config.InputHeight  = mEisInput_H;
                mEisAlgoProcData.eis_image_size_config.TargetWidth  = mP1Target_W;
                mEisAlgoProcData.eis_image_size_config.TargetHeight = mP1Target_H;
            }

            EIS_LOG("EisIn(%u,%u),P1T(%u,%u)",mEisInput_W,mEisInput_H,mP1Target_W,mP1Target_H);

            //> get EIS HW statistic

            if(EIS_RETURN_EISO_MISS == m_pEisDrv->GetEisHwStatistic(&mEisAlgoProcData.eis_state,aTimeStamp))
            {
                EIS_WRN("EISO data miss");

                return EIS_RETURN_NO_ERROR;
            }

            if(g_debugDump == 3)
            {
                DumpStatistic(mEisAlgoProcData.eis_state);
            }

            //> get EIS HW setting of eis_op_vert and eis_op_hori

            mEisAlgoProcData.DivH = m_pEisDrv->GetEisDivH();
            mEisAlgoProcData.DivV = m_pEisDrv->GetEisDivV();

            //> get MB number

            mEisAlgoProcData.EisWinNum = m_pEisDrv->GetEisMbNum();

            //> get Acc & Gyro info

            mEisAlgoProcData.sensor_info.GyroValid = mGyroEnable;
            mEisAlgoProcData.sensor_info.Gvalid    = mAccEnable;

            for(MUINT32 i = 0; i < 3; i++)
            {
                mEisAlgoProcData.sensor_info.AcceInfo[i] = gAccInfo[i];
                mEisAlgoProcData.sensor_info.GyroInfo[i] = gGyroInfo[i];
            }

            if(g_debugDump >= 1)
            {
                EIS_LOG("EN:(Acc,Gyro)=(%d,%d)",mAccEnable,mGyroEnable);
                EIS_LOG("EIS:Acc(%f,%f,%f)",mEisAlgoProcData.sensor_info.AcceInfo[0],mEisAlgoProcData.sensor_info.AcceInfo[1],mEisAlgoProcData.sensor_info.AcceInfo[2]);
                EIS_LOG("EIS:Gyro(%f,%f,%f)",mEisAlgoProcData.sensor_info.GyroInfo[0],mEisAlgoProcData.sensor_info.GyroInfo[1],mEisAlgoProcData.sensor_info.GyroInfo[2]);
            }

            //====== EIS Algorithm ======

            err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SET_PROC_INFO, &mEisAlgoProcData, NULL);
            if(err != S_EIS_OK)
            {
                EIS_ERR("EisAlg:EIS_FEATURE_SET_PROC_INFO fail(0x%x)",err);
                err = EIS_RETURN_API_FAIL;
                return err;
            }

            err = m_pEisAlg->EisMain(&eisCMVResult);
            if(err != S_EIS_OK)
            {
                EIS_ERR("EisAlg:EisMain fail(0x%x)",err);
                err = EIS_RETURN_API_FAIL;
                return err;
            }

            //====== Get EIS Result to EIS Plus ======

            EIS_GET_PLUS_INFO_STRUCT eisData2EisPlus;

            err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_EIS_PLUS_DATA, NULL, &eisData2EisPlus);
            if(err != S_EIS_OK)
            {
                EIS_ERR("EisAlg:EIS_FEATURE_GET_EIS_PLUS_DATA fail(0x%x)",err);
                err = EIS_RETURN_API_FAIL;
                return err;
            }

            {
                Mutex::Autolock lock(mP2Lock);

                if(m_pEisDrv->Get2PixelMode())
                {
                    if(g_debugDump > 0)
                    {
                        EIS_LOG("eisData2EisPlus.GMVx *= 2");
                    }
                    eisData2EisPlus.GMVx *= 2.0;
                }

                mEis2EisPlusGmvX.push(eisData2EisPlus.GMVx);
                mEis2EisPlusGmvY.push(eisData2EisPlus.GMVy);
                mEis2EisPlusConfX.push(eisData2EisPlus.ConfX);
                mEis2EisPlusConfY.push(eisData2EisPlus.ConfY);
            }

            //====== Get GMV ======

            EIS_GMV_INFO_STRUCT eisGMVResult;

            err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_ORI_GMV, NULL, &eisGMVResult);
            if(err != S_EIS_OK)
            {
                EIS_ERR("EisAlg:EIS_FEATURE_GET_ORI_GMV fail(0x%x)",err);
                err = EIS_RETURN_API_FAIL;
                return err;
            }

            //====== Save EIS CMV and GMV =======

            if(m_pEisDrv->Get2PixelMode())
            {
                if(g_debugDump > 0)
                {
                    EIS_LOG("eisGMVResult.EIS_GMVx *= 2");
                }

                eisGMVResult.EIS_GMVx *= 2;
            }

            mGMV_X = eisGMVResult.EIS_GMVx;
            mGMV_Y = eisGMVResult.EIS_GMVy;

            //====== Prepare EIS Result ======

            PrepareEisResult(eisCMVResult.CMV_X,eisCMVResult.CMV_Y,eisData2EisPlus.ConfX,eisData2EisPlus.ConfY,aTimeStamp);

            //====== Get First Frame Info ======

            mFrameCnt = m_pEisDrv->GetFirstFrameInfo();

            if(g_debugDump >= 1)
            {
                EIS_LOG("mFrameCnt(%u)",mFrameCnt);
            }

            //====== Not The First Frame ======

            if(mFrameCnt == 0)
            {
                EIS_LOG("not first frame");
                mFrameCnt = 1;

                // move to EIS_DRV to handle this
            #if 0
                err = m_pEisDrv->SetFirstFrame(0);
                if(err != EIS_RETURN_NO_ERROR)
                {
                    EIS_ERR("set first frame fail(0x%08x)",err);
                }
                else
                {
                     mFrameCnt = 1;
                }
            #endif
            }

            //====== Dynamic Debug ======

            if(g_debugDump >= 1 && mFrameCnt < DEBUG_DUMP_FRAMW_NUM)
            {
                if(g_debugDump == 3)
                {
                    m_pEisDrv->DumpReg(EIS_PASS_1);
                }

                if(mEisPass1Only == 1)
                {
                    ++mFrameCnt;
                }
            }

    #endif
        }
    }
    else if(aEisPass == EIS_PASS_2)
    {
        if(mEisPass1Only == 1)
        {
            EIS_WRN("Only use EIS pass1");
            return EIS_RETURN_NO_ERROR;
        }
        else
        {
#if EIS_ALGO_READY

            //====== Check Config Data ======

            if(apEisConfig == NULL)
            {
                EIS_ERR("apEisConfig is NULL");
                err = EIS_RETURN_NULL_OBJ;
                return err;
            }

            //====== Setting EIS Plus Algo Process Data ======

            EIS_GET_PLUS_INFO_STRUCT eisDataForEisPlus;
            EIS_PLUS_SET_PROC_INFO_STRUCT eisPlusProcData;

            {
                Mutex::Autolock lock(mP2Lock);

                if(mEis2EisPlusGmvX.empty() ||
                   mEis2EisPlusGmvY.empty() ||
                   mEis2EisPlusConfX.empty() ||
                   mEis2EisPlusConfY.empty())
                {
                    EIS_ERR("empty queue(%d,%d,%d,%d)",mEis2EisPlusGmvX.size(),
                                                       mEis2EisPlusGmvY.size(),
                                                       mEis2EisPlusConfX.size(),
                                                       mEis2EisPlusConfY.size());
                    err = EIS_RETURN_NULL_OBJ;
                    return err;
                }
                else
                {

                    eisDataForEisPlus.GMVx  = mEis2EisPlusGmvX.front();
                    eisDataForEisPlus.GMVy  = mEis2EisPlusGmvY.front();
                    eisDataForEisPlus.ConfX = mEis2EisPlusConfX.front();
                    eisDataForEisPlus.ConfY = mEis2EisPlusConfY.front();

                    mEis2EisPlusGmvX.pop();
                    mEis2EisPlusGmvY.pop();
                    mEis2EisPlusConfX.pop();
                    mEis2EisPlusConfY.pop();
                }
            }

            if(g_debugDump >= 1)
            {
                EIS_LOG("eisDataForEisPlus.GMVx(%f)",eisDataForEisPlus.GMVx);
                EIS_LOG("eisDataForEisPlus.GMVy(%f)",eisDataForEisPlus.GMVy);
                EIS_LOG("eisDataForEisPlus.ConfX(%d)",eisDataForEisPlus.ConfX);
                EIS_LOG("eisDataForEisPlus.ConfY(%d)",eisDataForEisPlus.ConfY);
            }

            //> Set EisPlusProcData

            eisPlusProcData.eis_info.eis_gmv_conf[0] = eisDataForEisPlus.ConfX;
            eisPlusProcData.eis_info.eis_gmv_conf[1] = eisDataForEisPlus.ConfY;
            eisPlusProcData.eis_info.eis_gmv[0]      = eisDataForEisPlus.GMVx;
            eisPlusProcData.eis_info.eis_gmv[1]      = eisDataForEisPlus.GMVy;

            //> get FE block number

            MUINT32 feBlockNum = 0;

            if(apEisConfig->srzOutW <= D1_WIDTH && apEisConfig->srzOutH <= D1_HEIGHT)
            {
                feBlockNum = 8;
            }
            else if(apEisConfig->srzOutW <= EIS_FE_MAX_INPUT_W && apEisConfig->srzOutH <= EIS_FE_MAX_INPUT_H)
            {
                feBlockNum = 16;
            }
            else
            {
                feBlockNum = 32;
                EIS_WRN("FE should be disabled");
            }

            eisPlusProcData.block_size   = feBlockNum;
            eisPlusProcData.imgiWidth    = apEisConfig->imgiW;
            eisPlusProcData.imgiHeight   = apEisConfig->imgiH;
            eisPlusProcData.CRZoWidth    = apEisConfig->crzOutW;
            eisPlusProcData.CRZoHeight   = apEisConfig->crzOutH;
            eisPlusProcData.SRZoWidth    = apEisConfig->srzOutW;
            eisPlusProcData.SRZoHeight   = apEisConfig->srzOutH;
            eisPlusProcData.oWidth       = apEisConfig->feTargetW;
            eisPlusProcData.oHeight      = apEisConfig->feTargetH;
            eisPlusProcData.TargetWidth  = apEisConfig->gpuTargetW;
            eisPlusProcData.TargetHeight = apEisConfig->gpuTargetH;
            eisPlusProcData.cropX        = apEisConfig->cropX;
            eisPlusProcData.cropY        = apEisConfig->cropY;

            //> config EIS Plus data

            mSrzOutW = apEisConfig->srzOutW;
            mSrzOutH = apEisConfig->srzOutH;

            //> set FE block number to driver

            m_pEisDrv->SetFeBlockNum(mSrzOutW,mSrzOutH,feBlockNum);

            if(g_debugDump >= 1)
            {
                EIS_LOG("mImgi(%u,%u)",eisPlusProcData.imgiWidth,eisPlusProcData.imgiHeight);
                EIS_LOG("CrzOut(%u,%u)",eisPlusProcData.CRZoWidth,eisPlusProcData.CRZoHeight);
                EIS_LOG("SrzOut(%u,%u)",eisPlusProcData.SRZoWidth,eisPlusProcData.SRZoHeight);
                EIS_LOG("FeTarget(%u,%u)",eisPlusProcData.oWidth,eisPlusProcData.oHeight);
                EIS_LOG("GpuTarget(%u,%u)",eisPlusProcData.TargetWidth,eisPlusProcData.TargetHeight);
                EIS_LOG("mCrop(%u,%u)",eisPlusProcData.cropX,eisPlusProcData.cropY);
                EIS_LOG("feBlockNum(%u)",feBlockNum);
            }

            //> get FEO statistic

            GetFeoStatistic();

            eisPlusProcData.fe_info.FE_X     = mFeoStatData.feX;
            eisPlusProcData.fe_info.FE_Y     = mFeoStatData.feY;
            eisPlusProcData.fe_info.FE_RES   = mFeoStatData.feRes;
            eisPlusProcData.fe_info.FE_DES   = mFeoStatData.feDes;
            eisPlusProcData.fe_info.FE_VALID = mFeoStatData.feValid;

            if(g_debugDump >= 1)
            {
                EIS_LOG("eisPlusProcData");
                EIS_LOG("eis_gmv_conf[0](%d)",eisPlusProcData.eis_info.eis_gmv_conf[0]);
                EIS_LOG("eis_gmv_conf[1](%d)",eisPlusProcData.eis_info.eis_gmv_conf[1]);
                EIS_LOG("eis_gmv[0](%f)",eisPlusProcData.eis_info.eis_gmv[0]);
                EIS_LOG("eis_gmv[1](%f)",eisPlusProcData.eis_info.eis_gmv[1]);
                EIS_LOG("block_size(%d)",eisPlusProcData.block_size);
                EIS_LOG("imgi(%d,%d)",eisPlusProcData.imgiWidth,eisPlusProcData.imgiHeight);
                EIS_LOG("CRZ(%d,%d)",eisPlusProcData.CRZoWidth,eisPlusProcData.CRZoHeight);
                EIS_LOG("SRZ(%d,%d)",eisPlusProcData.SRZoWidth,eisPlusProcData.SRZoHeight);
                EIS_LOG("target(%d,%d)",eisPlusProcData.TargetWidth,eisPlusProcData.TargetHeight);
                EIS_LOG("crop(%d,%d)",eisPlusProcData.cropX,eisPlusProcData.cropY);
            }

            //====== EIS Plus Algorithm ======

            err = m_pEisPlusAlg->EisPlusFeatureCtrl(EIS_PLUS_FEATURE_SET_PROC_INFO,&eisPlusProcData, NULL);
            if(err != S_EIS_PLUS_OK)
            {
                EIS_ERR("EisPlus:EIS_PLUS_FEATURE_SET_PROC_INFO fail(0x%x)",err);
                err = EIS_RETURN_API_FAIL;
                return err;
            }

            err = m_pEisPlusAlg->EisPlusMain(&mEisPlusResult);
            if(err != S_EIS_PLUS_OK)
            {
                EIS_ERR("EisPlus:EisMain fail(0x%x)",err);
                err = EIS_RETURN_API_FAIL;
                return err;
            }

            //====== Dynamic Debug ======

            if(g_debugDump >= 1)
            {
                EIS_INF("EIS GPU WARP MAP");
                for(MUINT32  i = 0; i < mGpuGridW*mGpuGridH; ++i)
                {
                    EIS_LOG("X[%u]=%d",i,mEisPlusResult.GridX[i]);
                    EIS_LOG("Y[%u]=%d",i,mEisPlusResult.GridY[i]);
                }
            }

            if(g_debugDump >= 1 && mFrameCnt < DEBUG_DUMP_FRAMW_NUM)
            {
                if(g_debugDump == 3)
                {
                    m_pEisDrv->DumpReg(EIS_PASS_2);
                }
                ++mFrameCnt;
            }

 #endif
        }

    }

    if(g_debugDump >= 1)
    {
        EIS_LOG("-");
    }

    mDoEisCount++;    //Vent@20140427: Count how many times DoEis() is run. Add for EIS GMV Sync Check.

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::SetEisPlusGpuInfo(MINT32 * const aGridX, MINT32 * const aGridY)
{
    mEisPlusResult.GridX = aGridX;
    mEisPlusResult.GridY = aGridY;

    if(g_debugDump >= 1)
    {
        EIS_LOG("[IN]grid VA(0x%08x,0x%08x)",aGridX,aGridY);
        EIS_LOG("[MEMBER]grid VA(0x%08x,0x%08x)",mEisPlusResult.GridX,mEisPlusResult.GridY);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::SetEisP2User(char const *userName)
{
    EIS_LOG("%s(%d)",userName,++mEisP2UserCnt);
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::EisThreadStop()
{
    m_pEisDrv->SetEisoThreadState(EIS_SW_STATE_UNINIT);
}


/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::PrepareEisResult(const MINT32 &cmvX, const MINT32 &cmvY,const MINT32 &aGmvConfidX,const MINT32 &aGmvConfidY,const MINT64 &aTimeStamp)
{
    EIS_LOG("cmvX(%d),cmvY(%d)",cmvX,cmvY);

    Mutex::Autolock lock(mP1Lock);

    //====== Boundary Checking ======

    if(cmvX < 0)
    {
        EIS_ERR("cmvX should not be negative(%u), fix to 0",cmvX);

        mCmvX_Int = mCmvX_Flt = 0;
    }
    else
    {
        MFLOAT tempCMV_X = cmvX / 256.0;
        MINT32 tempFinalCmvX = cmvX;
        if((tempCMV_X + (MFLOAT)mP1Target_W) > (MFLOAT)mEisInput_W)
        {
            EIS_LOG("cmvX too large(%u), fix to %u",cmvX,(mEisInput_W - mP1Target_W));

            tempFinalCmvX = (mEisInput_W - mP1Target_W);
        }

        if(m_pEisDrv->Get2PixelMode())
        {
            if(g_debugDump > 0)
            {
                EIS_LOG("tempFinalCmvX *= 2");
            }

            tempFinalCmvX *= 2;
        }

        mCmvX_Int = (tempFinalCmvX & (~0xFF)) >> intPartShift;
        mCmvX_Flt = (tempFinalCmvX & (0xFF)) << floatPartShift;
    }

    if(cmvY < 0)
    {
        EIS_ERR("cmvY should not be negative(%u), fix to 0",cmvY);

        mCmvY_Int = mCmvY_Flt = 0;
    }
    else
    {
        MFLOAT tempCMV_Y = cmvY / 256.0;
        MINT32 tempFinalCmvY = cmvY;
        if((tempCMV_Y + (MFLOAT)mP1Target_H) > (MFLOAT)mEisInput_H)
        {
            EIS_LOG("cmvY too large(%u), fix to %u",cmvY,(mEisInput_H - mP1Target_H));

            tempFinalCmvY = (mEisInput_H - mP1Target_H);
        }

        mCmvY_Int = (tempFinalCmvY & (~0xFF)) >> intPartShift;
        mCmvY_Flt = (tempFinalCmvY & (0xFF)) << floatPartShift;
    }

    EIS_LOG("X(%u,%u),Y(%u,%u)",mCmvX_Int,mCmvX_Flt,mCmvY_Int,mCmvY_Flt);

    //====== Save for Pass2 User if Needed ======

    if(mEisP2UserCnt != 0)
    {
        SaveEisResultForP2(aGmvConfidX,aGmvConfidY,aTimeStamp);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::SaveEisResultForP2(const MINT32 &aGmvConfidX,const MINT32 &aGmvConfidY,const MINT64 &aTimeStamp)
{
    Mutex::Autolock lock(mLock);

    EIS_P1_RESULT_INFO eisP1Result;

    eisP1Result.DoEisCount = mDoEisCount;    //Vent@20140427: Add for EIS GMV Sync Check.

    eisP1Result.cmvX_Int   = mCmvX_Int;
    eisP1Result.cmvX_Flt   = mCmvX_Flt;
    eisP1Result.cmvY_Int   = mCmvY_Int;
    eisP1Result.cmvY_Flt   = mCmvY_Flt;
    eisP1Result.gmvX       = mGMV_X;
    eisP1Result.gmvY       = mGMV_Y;
    eisP1Result.gmvConfidX = aGmvConfidX;
    eisP1Result.gmvConfidY = aGmvConfidY;
    eisP1Result.timeStamp  = aTimeStamp;

    if(g_debugDump > 0)
    {
        EIS_LOG("Gmv(%d,%d),gmvConfid(%d,%d),Int(%d,%d),Flt(%d,%d),Count(%d),TS(%lld)",eisP1Result.gmvX,
                                                                                       eisP1Result.gmvY,
                                                                                       eisP1Result.gmvConfidX,
                                                                                       eisP1Result.gmvConfidY,
                                                                                       eisP1Result.cmvX_Int,
                                                                                       eisP1Result.cmvY_Int,
                                                                                       eisP1Result.cmvX_Flt,
                                                                                       eisP1Result.cmvY_Flt,
                                                                                       eisP1Result.DoEisCount,
                                                                                       eisP1Result.timeStamp);
    }

    mEisResultForP2.push(eisP1Result);

    if(mEisResultForP2.size() > 30)
    {
        EIS_LOG("too much unused data");
        while(mEisResultForP2.size() > 30)
        {
            mEisResultForP2.pop();
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::GetEisResult(MUINT32 &a_CMV_X_Int,
                                 MUINT32 &a_CMV_X_Flt,
                                 MUINT32 &a_CMV_Y_Int,
                                 MUINT32 &a_CMV_Y_Flt,
                                 MUINT32 &a_TarWidth,
                                 MUINT32 &a_TarHeight)
{
    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        a_CMV_X_Int = 0;
        a_CMV_X_Flt = 0;
        a_CMV_Y_Int = 0;
        a_CMV_Y_Flt = 0;
        a_TarWidth  = 0;
        a_TarHeight = 0;
        return;
    }

    {
        Mutex::Autolock lock(mP1Lock);

        a_CMV_X_Int = mCmvX_Int;
        a_CMV_X_Flt = mCmvX_Flt;
        a_CMV_Y_Int = mCmvY_Int;
        a_CMV_Y_Flt = mCmvY_Flt;
        a_TarWidth  = mP1Target_W;
        a_TarHeight = mP1Target_H;
    }

    if(g_debugDump >= 1)
    {
        EIS_LOG("X(%u,%u),Y(%u,%u)",a_CMV_X_Int,a_CMV_X_Flt,a_CMV_Y_Int,a_CMV_Y_Flt);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::GetEisPlusResult(P_EIS_PLUS_RESULT_INFO_STRUCT apEisPlusResult)
{
    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return;
    }

    apEisPlusResult->ClipX = mEisPlusResult.ClipX;
    apEisPlusResult->ClipY = mEisPlusResult.ClipY;

    if(g_debugDump >= 1)
    {
        EIS_LOG("Clip(%u,%u)",apEisPlusResult->ClipX,apEisPlusResult->ClipY);
    }
}


/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::GetEisGmv(MINT32 &aGMV_X, MINT32 &aGMV_Y, MUINT32 *aEisInW, MUINT32 *aEisInH)
{
    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return;
    }

    aGMV_X = mGMV_X;
    aGMV_Y = mGMV_Y;

    {
        Mutex::Autolock lock(mP1Lock);

        if(aEisInW != NULL)
        {
            *aEisInW = mEisInput_W;
            EIS_LOG("aEisInW(%u)",*aEisInW);
        }

        if(aEisInH != NULL)
        {
            *aEisInH = mEisInput_H;
            EIS_LOG("aEisInH(%u)",*aEisInH);
        }
    }

    if(g_debugDump >= 1)
    {
        EIS_LOG("GMV(%d,%d)",aGMV_X,aGMV_Y);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::GetFeoRegInfo(FEO_REG_INFO *aFeoRegInfo)
{
    if(mEisSupport == MFALSE)
    {
        EIS_LOG("mSensorIdx(%u) not support EIS",mSensorIdx);
        return;
    }

    FEO_COFIG_DATA feoRegInfo;

    m_pEisDrv->GetFeoRegInfo(&feoRegInfo);

    aFeoRegInfo->xSize       = feoRegInfo.xSize;
    aFeoRegInfo->ySize       = feoRegInfo.ySize;
    aFeoRegInfo->stride      = feoRegInfo.stride;
    aFeoRegInfo->memID       = feoRegInfo.memInfo.memID;
    aFeoRegInfo->size        = feoRegInfo.memInfo.size;
    aFeoRegInfo->va          = feoRegInfo.memInfo.virtAddr;
    aFeoRegInfo->pa          = feoRegInfo.memInfo.phyAddr;
    aFeoRegInfo->bufSecu     = feoRegInfo.memInfo.bufSecu;
    aFeoRegInfo->bufCohe     = feoRegInfo.memInfo.bufCohe;
    aFeoRegInfo->useNoncache = feoRegInfo.memInfo.useNoncache;

    if(g_debugDump >= 1)
    {
        EIS_LOG("W(%u),H(%u)",aFeoRegInfo->inputW,aFeoRegInfo->inputH);
        EIS_LOG("X(%u),Y(%u),STRIDE(%u)",aFeoRegInfo->xSize,aFeoRegInfo->ySize,aFeoRegInfo->stride);
        EIS_LOG("feo: ID(%d),size(%u),VA(0x%8x),PA(0x%8x),Secu(%d),Cohe(%d),Noncache(%d)",aFeoRegInfo->memID,
                                                                                          aFeoRegInfo->size,
                                                                                          aFeoRegInfo->va,
                                                                                          aFeoRegInfo->pa,
                                                                                          aFeoRegInfo->bufCohe,
                                                                                          aFeoRegInfo->bufSecu,
                                                                                          aFeoRegInfo->useNoncache);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::GetEisPlusGpuInfo(EIS_PLUS_GPU_INFO *aEisPlusGpu)
{
    aEisPlusGpu->gridW = mGpuGridW;
    aEisPlusGpu->gridH = mGpuGridH;

    if(g_debugDump >= 1)
    {
        EIS_LOG("W(%u),H(%u)",aEisPlusGpu->gridW,aEisPlusGpu->gridH);
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL EisHalImp_R::GetEisSupportInfo(const MUINT32 &aSensorIdx)
{
    mEisSupport = m_pEisDrv->GetEisSupportInfo(aSensorIdx);
    return mEisSupport;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::GetFeoStatistic()
{
    m_pEisDrv->GetFeoStatistic(&mFeoStatData);
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 EisHalImp_R::GetEisPlusCropRatio()
{
    if(g_debugDump >= 1)
    {
        EIS_LOG("mEisPlusCropRatio(%d)",mEisPlusCropRatio);
    }
    return mEisPlusCropRatio;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::GetEisInfoAtP2(char const *userName,EIS_P1_RESULT_INFO *apEisResult,const MINT64 &aTimeStamp,MBOOL lastUser)
{
    if(mEisP2UserCnt == 0)
    {
        EIS_LOG("No P2 user");
    }
    else if(mEisResultForP2.empty())
    {
        EIS_ERR("(%s)mEisResultForP2 is empty",userName);
        apEisResult->timeStamp = EIS_EISO_SYNC_FAIL;
    }
    else
    {
        EIS_LOG("%s(%d)",userName,lastUser);

        Mutex::Autolock lock(mLock);

        MUINT32 hasMatch = 0;

        while(!mEisResultForP2.empty())
        {
            if(mEisResultForP2.front().timeStamp < aTimeStamp)
            {
                EIS_LOG("drop,TS(%lld)",mEisResultForP2.front().timeStamp);
                mEisResultForP2.pop();
            }
            else if(mEisResultForP2.front().timeStamp == aTimeStamp)
            {
                hasMatch = 1;
                break;
            }
            else
            {
                hasMatch = 0;
                break;
            }
        }

        if(hasMatch == 1)
        {
            apEisResult->cmvX_Int   = mEisResultForP2.front().cmvX_Int;
            apEisResult->cmvX_Flt   = mEisResultForP2.front().cmvX_Flt;
            apEisResult->cmvY_Int   = mEisResultForP2.front().cmvY_Int;
            apEisResult->cmvY_Flt   = mEisResultForP2.front().cmvY_Flt;
            apEisResult->gmvX       = mEisResultForP2.front().gmvX;
            apEisResult->gmvY       = mEisResultForP2.front().gmvY;
            apEisResult->DoEisCount = mEisResultForP2.front().DoEisCount;
            apEisResult->gmvConfidX = mEisResultForP2.front().gmvConfidX;
            apEisResult->gmvConfidY = mEisResultForP2.front().gmvConfidY;
            apEisResult->timeStamp  = mEisResultForP2.front().timeStamp;

            if(g_debugDump > 0)
            {
                EIS_LOG("match,TS(%lld)",mEisResultForP2.front().timeStamp);
            }

            if(lastUser == MTRUE)
            {
                mEisResultForP2.pop();
            }
        }
        else
        {
            apEisResult->timeStamp = EIS_EISO_SYNC_FAIL;
            EIS_ERR("no match");
        }

        if(g_debugDump > 0)
        {
            EIS_LOG("cmvX(%u,%u),cmvY(%u,%u),GMV(%d,%d),Count(%d),Confid(%d,%d),TS(%lld)",apEisResult->cmvX_Int,
                                                                                          apEisResult->cmvX_Flt,
                                                                                          apEisResult->cmvY_Int,
                                                                                          apEisResult->cmvY_Flt,
                                                                                          apEisResult->gmvX,
                                                                                          apEisResult->gmvY,
                                                                                          apEisResult->DoEisCount,
                                                                                          apEisResult->gmvConfidX,
                                                                                          apEisResult->gmvConfidY,
                                                                                          apEisResult->timeStamp);
        }
    }
}

MSize EisHalImp_R::QueryMinSize(MBOOL isEISOn, MSize sensorSize, MSize requestSize)
{
    MSize retSize;
    MUINT32 out_width;
    MUINT32 out_height;
    if (MFALSE == isEISOn)
    {
        out_width = (requestSize.w <= 160)? 160 : requestSize.w;
        out_height = (requestSize.h <= 160)? 160 : requestSize.h;
    }else
    {
        out_width = (requestSize.w <= HD_720P_WIDTH)? HD_720P_WIDTH : requestSize.w;
        out_height = (requestSize.h <= HD_720P_HEIGHT)? HD_720P_HEIGHT : requestSize.h;
    }
    retSize = MSize(out_width,out_height);


    return retSize;
}


/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::FlushMemory(const EIS_DMA_ENUM &aDma,const EIS_FLUSH_ENUM &aFlush)
{
    m_pEisDrv->FlushMemory(aDma,aFlush);
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::SendCommand(EIS_CMD_ENUM aCmd,MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    switch(aCmd)
    {
        case EIS_CMD_SET_STATE:
            m_pEisDrv->SetEisoThreadState((EIS_SW_STATE_ENUM)arg1);
            break;
        case EIS_CMD_CHECK_STATE:
            m_pEisDrv->SetEisoThreadState((EIS_SW_STATE_ENUM)arg1);
            break;
        default:
            EIS_ERR("wrong cmd(%d)",aCmd);
    }
}

#if EIS_ALGO_READY

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::GetEisStatistic(EIS_STATISTIC_STRUCT *a_pEIS_Stat)
{
    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        a_pEIS_Stat->i4LMV_X[i]    = mEisAlgoProcData.eis_state.i4LMV_X[i];
        a_pEIS_Stat->i4LMV_Y[i]    = mEisAlgoProcData.eis_state.i4LMV_Y[i];
        a_pEIS_Stat->i4LMV_X2[i]   = mEisAlgoProcData.eis_state.i4LMV_X2[i];
        a_pEIS_Stat->i4LMV_Y2[i]   = mEisAlgoProcData.eis_state.i4LMV_Y2[i];
        a_pEIS_Stat->NewTrust_X[i] = mEisAlgoProcData.eis_state.NewTrust_X[i];
        a_pEIS_Stat->NewTrust_Y[i] = mEisAlgoProcData.eis_state.NewTrust_Y[i];
        a_pEIS_Stat->SAD[i]        = mEisAlgoProcData.eis_state.SAD[i];
        a_pEIS_Stat->SAD2[i]       = mEisAlgoProcData.eis_state.SAD2[i];
        a_pEIS_Stat->AVG_SAD[i]    = mEisAlgoProcData.eis_state.AVG_SAD[i];
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::GetEisCustomize(EIS_TUNING_PARA_STRUCT *a_pDataOut)
{
    if(g_debugDump >= 1)
    {
        EIS_LOG("+");
    }

    EIS_Customize_Para_t customSetting;

    get_EIS_CustomizeData(&customSetting);

    a_pDataOut->sensitivity            = (EIS_SENSITIVITY_ENUM)customSetting.sensitivity;
    a_pDataOut->filter_small_motion    = customSetting.filter_small_motion;
    a_pDataOut->adv_shake_ext          = customSetting.adv_shake_ext;  // 0 or 1
    a_pDataOut->stabilization_strength = customSetting.stabilization_strength;  // 0.5~0.95

    a_pDataOut->advtuning_data.new_tru_th        = customSetting.new_tru_th;         // 0~100
    a_pDataOut->advtuning_data.vot_th            = customSetting.vot_th;             // 1~16
    a_pDataOut->advtuning_data.votb_enlarge_size = customSetting.votb_enlarge_size;  // 0~1280
    a_pDataOut->advtuning_data.min_s_th          = customSetting.min_s_th;           // 10~100
    a_pDataOut->advtuning_data.vec_th            = customSetting.vec_th;             // 0~11   should be even
    a_pDataOut->advtuning_data.spr_offset        = customSetting.spr_offset;         //0 ~ MarginX/2
    a_pDataOut->advtuning_data.spr_gain1         = customSetting.spr_gain1;          // 0~127
    a_pDataOut->advtuning_data.spr_gain2         = customSetting.spr_gain2;          // 0~127

    a_pDataOut->advtuning_data.gmv_pan_array[0] = customSetting.gmv_pan_array[0];   //0~5
    a_pDataOut->advtuning_data.gmv_pan_array[1] = customSetting.gmv_pan_array[1];   //0~5
    a_pDataOut->advtuning_data.gmv_pan_array[2] = customSetting.gmv_pan_array[2];   //0~5
    a_pDataOut->advtuning_data.gmv_pan_array[3] = customSetting.gmv_pan_array[3];   //0~5

    a_pDataOut->advtuning_data.gmv_sm_array[0] = customSetting.gmv_sm_array[0];    //0~5
    a_pDataOut->advtuning_data.gmv_sm_array[1] = customSetting.gmv_sm_array[1];    //0~5
    a_pDataOut->advtuning_data.gmv_sm_array[2] = customSetting.gmv_sm_array[2];    //0~5
    a_pDataOut->advtuning_data.gmv_sm_array[3] = customSetting.gmv_sm_array[3];    //0~5

    a_pDataOut->advtuning_data.cmv_pan_array[0] = customSetting.cmv_pan_array[0];   //0~5
    a_pDataOut->advtuning_data.cmv_pan_array[1] = customSetting.cmv_pan_array[1];   //0~5
    a_pDataOut->advtuning_data.cmv_pan_array[2] = customSetting.cmv_pan_array[2];   //0~5
    a_pDataOut->advtuning_data.cmv_pan_array[3] = customSetting.cmv_pan_array[3];   //0~5

    a_pDataOut->advtuning_data.cmv_sm_array[0] = customSetting.cmv_sm_array[0];    //0~5
    a_pDataOut->advtuning_data.cmv_sm_array[1] = customSetting.cmv_sm_array[1];    //0~5
    a_pDataOut->advtuning_data.cmv_sm_array[2] = customSetting.cmv_sm_array[2];    //0~5
    a_pDataOut->advtuning_data.cmv_sm_array[3] = customSetting.cmv_sm_array[3];    //0~5

    a_pDataOut->advtuning_data.vot_his_method  = (EIS_VOTE_METHOD_ENUM)customSetting.vot_his_method; //0 or 1
    a_pDataOut->advtuning_data.smooth_his_step = customSetting.smooth_his_step; // 2~6

    a_pDataOut->advtuning_data.eis_debug = customSetting.eis_debug;

    if(g_debugDump >= 1)
    {
        EIS_LOG("-");
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::GetEisPlusCustomize(EIS_PLUS_TUNING_PARA_STRUCT *a_pTuningData)
{
    if(g_debugDump >= 1)
    {
        EIS_LOG("+");
    }

    EIS_PLUS_Customize_Para_t customSetting;

    get_EIS_PLUS_CustomizeData(&customSetting);

    a_pTuningData->warping_mode           = static_cast<MINT32>(customSetting.warping_mode);
    a_pTuningData->effort                 = 2;  // limit to 400 points
    a_pTuningData->search_range_x         = customSetting.search_range_x;
    a_pTuningData->search_range_y         = customSetting.search_range_y;
    a_pTuningData->crop_ratio             = customSetting.crop_ratio;
    a_pTuningData->stabilization_strength = customSetting.stabilization_strength;

    if(g_debugDump >= 1)
    {
        EIS_LOG("-");
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp_R::DumpStatistic(const EIS_STATISTIC_STRUCT &aEisStat)
{
    EIS_LOG("+");

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF("MB%d%d,(LMV_X,LMV_Y)=(%d,%d)",(i/4),(i%4),aEisStat.i4LMV_X[i],aEisStat.i4LMV_Y[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF("MB%d%d,(LMV_X2,LMV_Y2)=(%d,%d)",(i/4),(i%4),aEisStat.i4LMV_X2[i],aEisStat.i4LMV_Y2[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF("MB%d%d,MinSAD(%u)",(i/4),(i%4),aEisStat.SAD[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF("MB%d%d,(NewTrust_X,NewTrust_Y)=(%u,%u)",(i/4),(i%4),aEisStat.NewTrust_X[i],aEisStat.NewTrust_Y[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF("MB%d%d,MinSAD2(%u)",(i/4),(i%4),aEisStat.SAD2[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF("MB%d%d,AvgSAD(%u)",(i/4),(i%4),aEisStat.AVG_SAD[i]);
    }

    EIS_LOG("-");
}

#endif

