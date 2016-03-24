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
 * @file pip_hal.cpp
 * @brief PIP Hal Source File
 * @details PIP Hal Source File
 */
#define LOG_TAG "PipHal"
#define MTK_LOG_ENABLE 1
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

#include <android/sensor.h>

using namespace android;


#include "mtkcam/hal/IHalSensor.h"     // (Under mediatek\hardware\include\) For SensorStaticInfo struct.

using namespace NSCam;

#include "pip_hal_imp.h"

#include <mtkcam/v1/camutils/CamInfo.h> // For queryHalSensorDev().

/*******************************************************************************
*
********************************************************************************/
#define PIP_HAL_DEBUG

#ifdef PIP_HAL_DEBUG    // Enable debug log.

#undef __func__
#define __func__ __FUNCTION__

#define PIP_DBG(fmt, arg...)    ALOGD("[%s]" fmt, __func__, ##arg)
#define PIP_INF(fmt, arg...)    ALOGI("[%s]" fmt, __func__, ##arg)
#define PIP_WRN(fmt, arg...)    ALOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define PIP_ERR(fmt, arg...)    ALOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else   // Disable debug log.
#define PIP_DBG(a,...)
#define PIP_INF(a,...)
#define PIP_WRN(a,...)
#define PIP_ERR(a,...)
#endif  // PIP_HAL_DEBUG



/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/
#define PIP_SENSOR_SIZE_20M   (20971520 + 1048576)    // (20 * 1024 * 1024) * (100% + 5%)
#define PIP_SENSOR_SIZE_16M   (16777216 + 838861)     // (16 * 1024 * 1024) * (100% + 5%)
#define PIP_SENSOR_SIZE_13M   (13631488 + 681574)     // (13 * 1024 * 1024) * (100% + 5%)
#define PIP_SENSOR_SIZE_8M    ( 8388608 + 419430)     // ( 8 * 1024 * 1024) * (100% + 5%)
#define PIP_SENSOR_SIZE_5M    ( 5242880 + 262144)     // ( 5 * 1024 * 1024) * (100% + 5%)

/**************************************************************************
 *     E N U M / S T R U C T / T Y P E D E F    D E C L A R A T I O N     *
 **************************************************************************/

/**************************************************************************
 *                 E X T E R N A L    R E F E R E N C E S                 *
 **************************************************************************/

/**************************************************************************
 *                         G L O B A L    D A T A                         *
 **************************************************************************/

/**************************************************************************
 *       P R I V A T E    F U N C T I O N    D E C L A R A T I O N        *
 **************************************************************************/




/*******************************************************************************
*
********************************************************************************/
PipHal *PipHal::createInstance(const MUINT32 &aSensorIdx)
{
    PipHal *pPipHal = PipHalImp::getInstance(aSensorIdx);
    pPipHal->Init();

    return pPipHal;
}

/*******************************************************************************
*
********************************************************************************/
PipHal *PipHalImp::getInstance(const MUINT32 &aSensorIdx)
{
    PIP_DBG("sensorIdx(%u)",aSensorIdx);

    switch(aSensorIdx)
    {
        case 0 : return PipHalObj<0>::getInstance();
        case 1 : return PipHalObj<1>::getInstance();
        case 2 : return PipHalObj<2>::getInstance();
        case 3 : return PipHalObj<3>::getInstance();
        default :
            PIP_WRN("sensor count is limit to 4 sensors, use 0");
            return PipHalObj<0>::getInstance();
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID PipHalImp::destroyInstance(char const *userName)
{
    PIP_DBG("%s",userName);
    Uninit();
}

/*******************************************************************************
*
********************************************************************************/
PipHalImp::PipHalImp(const MUINT32 &aSensorIdx) : PipHal()
{
    mUsers = 0;

    //> member variable
    mSensorIdx = aSensorIdx;


    // sensor
    m_pHalSensorList = NULL;
    m_pHalSensor = NULL;


}

/*******************************************************************************
*
********************************************************************************/
MBOOL PipHalImp::Init()
{
    //====== Check Reference Count ======

    Mutex::Autolock lock(mLock);

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        PIP_DBG("snesorIdx(%u) has %d users", mSensorIdx, mUsers);
        return MTRUE;
    }


    PIP_DBG("mSensorIdx(%u) init", mSensorIdx);

    //====== Create Sensor Object ======
    m_pHalSensorList = IHalSensorList::get();
    if(m_pHalSensorList == NULL)
    {
        PIP_ERR("IHalSensorList::get fail");
        goto create_fail_exit;
    }



    //====== Increase User Count ======
    android_atomic_inc(&mUsers);

create_fail_exit:
    PIP_DBG("-");
    return MFALSE;

}

/*******************************************************************************
*
********************************************************************************/
MBOOL PipHalImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        PIP_DBG("mSensorIdx(%u) has 0 user",mSensorIdx);
        return MTRUE;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)    // there is no user
    {
        PIP_DBG("mSensorIdx(%u) uninit", mSensorIdx);


        //====== Destroy Sensor Object ======

        if(m_pHalSensorList != NULL)
        {
            m_pHalSensorList = NULL;
        }


        //======  Reset Member Variable ======
    }
    else
    {
        PIP_DBG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL PipHalImp::GetHwMaxFrameRate(MINT32 i4SensorDev, MUINT32 &ZsdHwMaxFrameRate, MUINT32 &NonZsdHwMaxFrameRate)
{
    // GetMaxFrameRate() will get both Main and Sub sensor HwMaxFrameRate, so i4CurrSensorDev is only for reference.
    MINT32 i4CurrSensorDev = (MINT32)m_pHalSensorList->querySensorDevIdx(mSensorIdx);    // Use current sensor idx to get device id. // Marked because we want to calculate specified sensor device.

    SensorStaticInfo rSensorStaticInfo;
    m_pHalSensorList->querySensorStaticInfo(i4SensorDev, &rSensorStaticInfo);

    // Get sensor full range
    MINT32 i4SensorPreviewWidth     = rSensorStaticInfo.previewWidth;
    MINT32 i4SensorPreviewHeight    = rSensorStaticInfo.previewHeight;
    MINT32 i4SensorFullWidth        = rSensorStaticInfo.captureWidth;
    MINT32 i4SensorFullHeight       = rSensorStaticInfo.captureHeight;
    MINT32 ai4SensorSize[2] = { // 0: Non-ZSD (PV), 1: ZSD (Full).
            i4SensorPreviewWidth * i4SensorPreviewHeight,
            i4SensorFullWidth    * i4SensorFullHeight
    };
   MUINT32 aiHwMaxFrameRate[2]; // 0: Non-ZSD (PV), 1: ZSD (Full).

    // Get RAW or YUV
    MINT32 i4SensorType = rSensorStaticInfo.sensorType;
    // Check Sensor Static Info.
    if (    // If value not reasonable, return fail.
        i4SensorType < 0 ||
        i4SensorDev < 0 ||
        i4SensorPreviewWidth < 0 ||
        i4SensorPreviewHeight < 0 ||
        i4SensorFullWidth < 0 ||
        i4SensorFullHeight < 0 ||
        ai4SensorSize[0] < 150 ||       // PV size
        ai4SensorSize[1] < 150          // Full size
    )
    {
        PIP_ERR("querySensorStaticInfo failed, use default Max NonZsd/Zsd value. Sensor Typ/InDev/CurrDev/Idx: (%d <RAW1YUV2>, %d, %d, %d), PV W/H: (%d, %d), Full W/H: (%d, %d).", i4SensorType, i4SensorDev, i4CurrSensorDev, mSensorIdx, i4SensorPreviewWidth, i4SensorPreviewHeight, i4SensorFullWidth, i4SensorFullHeight);
        return MFALSE;
    }

    for (MUINT32 i = 0; i < 2; i++) // 0: Non-ZSD (PV), 1: ZSD (Full).
    {
        if (i4SensorType == NSCam::SENSOR_TYPE_RAW) // RAW sensor.
        {
            if (i4SensorDev == SENSOR_DEV_MAIN) // Main Cam.
            {
                if (ai4SensorSize[i] > PIP_SENSOR_SIZE_16M)   // SensorSize > 16M. For this project, PIP only support 16M+5M@15fps. So main > 16M will return 0 (indicate error).
                {
                    aiHwMaxFrameRate[i] = 0;    // Not support this size, so frame rate return 0.
                }
                else if (ai4SensorSize[i] > PIP_SENSOR_SIZE_13M)   // 16M >= SensorSize > 13M
                {
                    aiHwMaxFrameRate[i] = 15;
                }
                else if (ai4SensorSize[i] > PIP_SENSOR_SIZE_8M)   // 13M >= SensorSize > 8M
                {
                    aiHwMaxFrameRate[i] = 15;
                }
                else    // 8M >= SensorSize
                {
                    aiHwMaxFrameRate[i] = 30;
                }
            }
            else if (i4SensorDev == SENSOR_DEV_SUB) // Sub Cam.
            {
                #ifdef USING_D1
                if (ai4SensorSize[i] > PIP_SENSOR_SIZE_5M)   // SensorSize > 5M. This project PIP only support 16M+5M. So sub > 5M will return 0 (indicate error).
                #else
                if (ai4SensorSize[i] > PIP_SENSOR_SIZE_8M)   // SensorSize > 8M. This project PIP only support 16M+8M. So sub > 8M will return 0 (indicate error).
                #endif
                {
                    aiHwMaxFrameRate[i] = 0;
                }
                else    // 5M >= SensorSize
                {
                    if (i == 0) // 0: Non-ZSD (PV)
                    {
                        aiHwMaxFrameRate[i] = 30;
                    }
                    else // 1: ZSD (Full).
                    {
                        aiHwMaxFrameRate[i] = 15;
                    }
                }
            }
        }
        else if (i4SensorType == NSCam::SENSOR_TYPE_YUV)    // YUV sensor.
        {
            if (i4SensorDev == SENSOR_DEV_MAIN) // Main Cam.
            {
                if (ai4SensorSize[i] > PIP_SENSOR_SIZE_16M)   // SensorSize > 16M. This project PIP only support 16M+5M. So main > 16M will return 0 (indicate error).
                {
                    aiHwMaxFrameRate[i] = 0;    // Not support this size, so frame rate return 0.
                }
                else if (ai4SensorSize[i] > PIP_SENSOR_SIZE_13M)   // 16M >= SensorSize > 13M
                {
                    aiHwMaxFrameRate[i] = 15;
                }
                else if (ai4SensorSize[i] > PIP_SENSOR_SIZE_8M)   // 13M >= SensorSize > 8M
                {
                    aiHwMaxFrameRate[i] = 15;
                }
                else    // 8M >= SensorSize
                {
                    aiHwMaxFrameRate[i] = 30;
                }
            }
            else if (i4SensorDev == SENSOR_DEV_SUB) // Sub Cam.
            {
                #ifdef USING_D1
                if (ai4SensorSize[i] > PIP_SENSOR_SIZE_5M)   // SensorSize > 5M. This project PIP only support 16M+5M. So sub > 5M will return 0 (indicate error).
                #else
                if (ai4SensorSize[i] > PIP_SENSOR_SIZE_8M)   // SensorSize > 8M. This project PIP only support 16M+8M. So sub > 8M will return 0 (indicate error).
                #endif
                {
                    aiHwMaxFrameRate[i] = 0;
                }
                else    // 5M >= SensorSize
                {
                    if (i == 0) // 0: Non-ZSD (PV)
                    {
                        aiHwMaxFrameRate[i] = 30;
                    }
                    else // 1: ZSD (Full).
                    {
                        aiHwMaxFrameRate[i] = 15;
                    }
                }
            }
        }
        else    // Unknown sensor type.
        {
            aiHwMaxFrameRate[i] = 0;
        }
    }

    // Output result.// 0: Non-ZSD (PV), 1: ZSD (Full).
    NonZsdHwMaxFrameRate = aiHwMaxFrameRate[0];
    ZsdHwMaxFrameRate    = aiHwMaxFrameRate[1];

    #ifdef USING_D1
    PIP_DBG("D1 Zsd/NonZsd HW Max FrmRate(%d, %d), Sensor Typ/InDev/CurrDev/Idx: (%d <RAW1YUV2>, %d <Main1Sub2>, %d, %d), PV W/H: (%d, %d), Full W/H: (%d, %d)", ZsdHwMaxFrameRate, NonZsdHwMaxFrameRate, i4SensorType, i4SensorDev, i4CurrSensorDev, mSensorIdx, i4SensorPreviewWidth, i4SensorPreviewHeight, i4SensorFullWidth, i4SensorFullHeight);
    #else
    PIP_DBG("Zsd/NonZsd HW Max FrmRate(%d, %d), Sensor Typ/InDev/CurrDev/Idx: (%d <RAW1YUV2>, %d <Main1Sub2>, %d, %d), PV W/H: (%d, %d), Full W/H: (%d, %d)", ZsdHwMaxFrameRate, NonZsdHwMaxFrameRate, i4SensorType, i4SensorDev, i4CurrSensorDev, mSensorIdx, i4SensorPreviewWidth, i4SensorPreviewHeight, i4SensorFullWidth, i4SensorFullHeight);
    #endif // USING_D3

    return MTRUE;   // No error.

}


/*******************************************************************************
*
********************************************************************************/
MBOOL PipHalImp::GetSensorMaxFrameRate(MINT32 i4SensorDev, MUINT32 &ZsdSensorMaxFrameRate, MUINT32 &NonZsdSensorMaxFrameRate)
{
    // GetMaxFrameRate() will get both Main and Sub sensor SensorMaxFrameRate, so i4CurrSensorDev is only for reference.
    MINT32 i4CurrSensorDev = (MINT32)m_pHalSensorList->querySensorDevIdx(mSensorIdx);    // Use current sensor idx to get device id. // Marked because we want to calculate specified sensor device.

    #if 1   // Use static info (sensor don't need be power-on.)
    SensorStaticInfo rSensorStaticInfo;
    m_pHalSensorList->querySensorStaticInfo(i4SensorDev, &rSensorStaticInfo);
    NonZsdSensorMaxFrameRate    = rSensorStaticInfo.previewFrameRate;  // 10base. Eg: 30fps --> 300
    ZsdSensorMaxFrameRate       = rSensorStaticInfo.captureFrameRate;  // 10base. Eg: 30fps --> 300
    #else   // Use sendCommand to get sensor info. (sensor must be power-on, or won't get any info.)
    // Sensor hal init
    IHalSensor *pHalSensorObj;
    pHalSensorObj = m_pHalSensorList->createSensor("pip_hal", mSensorIdx);
    if (pHalSensorObj == NULL) {
        PIP_ERR("Can not create SensorHal obj");
    }

    MUINT32 u4SensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    pHalSensorObj->sendCommand(
        i4SensorDev,
        SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
        (MINT32)&u4SensorMode,
        (MINT32)&ZsdSensorMaxFrameRate, // 10base. Eg: 30fps --> 300
        0
    );

    u4SensorMode = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
    pHalSensorObj->sendCommand(
        i4SensorDev,
        SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
        (MINT32)&u4SensorMode,
        (MINT32)&NonZsdSensorMaxFrameRate, // 10base. Eg: 30fps --> 300
        0
    );

    pHalSensorObj->destroyInstance("pip_hal");
    #endif  // Diff get sensor info method.

    // Calculate real fps. Eg: 300--->30fps.
    ZsdSensorMaxFrameRate    /= 10;
    NonZsdSensorMaxFrameRate /= 10;

    PIP_DBG("Zsd/NonZsd Sensor Max FrmRate(%d, %d), Sensor InDev/CurrDev/Idx: (%d <Main1Sub2>, %d, %d)", ZsdSensorMaxFrameRate, NonZsdSensorMaxFrameRate, i4SensorDev, i4CurrSensorDev, mSensorIdx);

    return MTRUE;   // No error.

}

/*******************************************************************************
*
********************************************************************************/
MBOOL PipHalImp::GetMaxFrameRate(MUINT32 &ZsdMaxFrameRate, MUINT32 &NonZsdMaxFrameRate)
{
    MINT32 i4CurrSensorDev = (MINT32)m_pHalSensorList->querySensorDevIdx(mSensorIdx);    // Use current sensor idx to get device id. // Marked because we want to calculate specified sensor device.


    // Get Sensor Max Frame Rate
    MUINT32 ZsdSensorMaxFrameRate_Main = 0, NonZsdSensorMaxFrameRate_Main = 0;
    MUINT32 ZsdSensorMaxFrameRate_Sub  = 0, NonZsdSensorMaxFrameRate_Sub  = 0;
    GetSensorMaxFrameRate(SENSOR_DEV_MAIN, ZsdSensorMaxFrameRate_Main, NonZsdSensorMaxFrameRate_Main);
    GetSensorMaxFrameRate(SENSOR_DEV_SUB,  ZsdSensorMaxFrameRate_Sub,  NonZsdSensorMaxFrameRate_Sub );

    // Get HW Max Frame Rate
    MUINT32 ZsdHwMaxFrameRate_Main = 0, NonZsdHwMaxFrameRate_Main = 0;
    MUINT32 ZsdHwMaxFrameRate_Sub  = 0, NonZsdHwMaxFrameRate_Sub  = 0;
    GetHwMaxFrameRate(SENSOR_DEV_MAIN, ZsdHwMaxFrameRate_Main, NonZsdHwMaxFrameRate_Main);
    GetHwMaxFrameRate(SENSOR_DEV_SUB,  ZsdHwMaxFrameRate_Sub,  NonZsdHwMaxFrameRate_Sub );

    MUINT32 ZsdMaxFrameRate_Main    = (ZsdSensorMaxFrameRate_Main    < ZsdHwMaxFrameRate_Main   ) ? (ZsdSensorMaxFrameRate_Main   ) : (ZsdHwMaxFrameRate_Main   );
    MUINT32 NonZsdMaxFrameRate_Main = (NonZsdSensorMaxFrameRate_Main < NonZsdHwMaxFrameRate_Main) ? (NonZsdSensorMaxFrameRate_Main) : (NonZsdHwMaxFrameRate_Main);

    MUINT32 ZsdMaxFrameRate_Sub    = (ZsdSensorMaxFrameRate_Sub    < ZsdHwMaxFrameRate_Sub   ) ? (ZsdSensorMaxFrameRate_Sub   ) : (ZsdHwMaxFrameRate_Sub   );
    MUINT32 NonZsdMaxFrameRate_Sub = (NonZsdSensorMaxFrameRate_Sub < NonZsdHwMaxFrameRate_Sub) ? (NonZsdSensorMaxFrameRate_Sub) : (NonZsdHwMaxFrameRate_Sub);

    #if 1   // Don't align Main/Sub frame rate (if don't want to do HW sync).
    if (i4CurrSensorDev == SENSOR_DEV_MAIN)
    {
        ZsdMaxFrameRate    = ZsdMaxFrameRate_Main;
        NonZsdMaxFrameRate = NonZsdMaxFrameRate_Main;
    }
    else if (i4CurrSensorDev == SENSOR_DEV_SUB)
    {
        ZsdMaxFrameRate    = ZsdMaxFrameRate_Sub;
        NonZsdMaxFrameRate = NonZsdMaxFrameRate_Sub;
    }
    #else   // Align Main/Sub frame rate (if want to do HW sync).
    ZsdMaxFrameRate    = (ZsdMaxFrameRate_Main    < ZsdMaxFrameRate_Sub   ) ? (ZsdMaxFrameRate_Main   ) : (ZsdMaxFrameRate_Sub   );
    NonZsdMaxFrameRate = (NonZsdMaxFrameRate_Main < NonZsdMaxFrameRate_Sub) ? (NonZsdMaxFrameRate_Main) : (NonZsdMaxFrameRate_Sub);
    #endif  // End

    PIP_DBG("Max Zsd/NonZsd FrmRate(%d, %d). CurrSensorDev(%d <Main1Sub2>)", ZsdMaxFrameRate, NonZsdMaxFrameRate, i4CurrSensorDev);

    return MTRUE;   // No error.

}

