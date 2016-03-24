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
#include "sensor_drv.h"
#include "seninf_drv.h"
//#include "HalSensorList.h"
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


/******************************************************************************
 *
 ******************************************************************************/
HalSensorList::EnumInfo const*
HalSensorList::
queryEnumInfoByIndex(MUINT index) const
{
    Mutex::Autolock _l(mEnumSensorMutex);
    //
    if  ( index >= mEnumSensorList.size() )
    {
        MY_LOGE("bad index:%d >= size:%d", index, mEnumSensorList.size());
        return  NULL;
    }
    //
    if  ( index != mEnumSensorList[index].muSensorIndex )
    {
        MY_LOGE("bad index:%d mismatch %d", index, mEnumSensorList[index].muSensorIndex);
        return  NULL;
    }
    //
    return  &mEnumSensorList[index];
}




/******************************************************************************
 *
 ******************************************************************************/
static
NSSensorType::Type
mapToSensorType(MUINT const sensor_type)
{
    NSSensorType::Type eSensorType;
    switch  (sensor_type)
    {
    case NSFeature::SensorInfoBase::EType_YUV:
        eSensorType = NSSensorType::eYUV;
        break;
    case NSFeature::SensorInfoBase::EType_RAW:
        eSensorType = NSSensorType::eRAW;
        break;
    default:
        eSensorType = NSSensorType::eRAW;
        break;
    }
    //
    return  eSensorType;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
HalSensorList::
enumerateSensor_Locked()
{
    int ret_count = 0;
    int ret = 0;

    //
    #warning "[WARN] Simulation for enumerateSensor_Locked()"

    MUINT halSensorDev = SENSOR_DEV_NONE;
    NSFeature::SensorInfoBase* pSensorInfo ;

    SensorDrv *const pSensorDrv = SensorDrv::get();
    SeninfDrv *const pSeninfDrv = SeninfDrv::createInstance();
    if(!pSeninfDrv) {
        MY_LOGE("pSeninfDrv == NULL");
                return 0;
    }


    ret = pSeninfDrv->init();
    if(ret < 0) {
        MY_LOGE("pSeninfDrv->init() fail");
                return 0;
    }
    pSeninfDrv->setMclk1(1, 1, 1, 0, 1, 0, 0);
    pSeninfDrv->setMclk2(1, 1, 1, 0, 1, 0, 0);
    //pSeninfDrv->setMclk3(1, 1, 1, 0, 1, 0, 0);  /* No main2 */

    int const iSensorsList = pSensorDrv->impSearchSensor(NULL);

    //query sensorinfo
    querySensorDrvInfo();
    //fill in metadata
    buildSensorMetadata();


    if  ( (iSensorsList & SENSOR_DEV_MAIN) == SENSOR_DEV_MAIN )
    {
        halSensorDev = SENSOR_DEV_MAIN;
        pSensorInfo = pSensorDrv->getMainSensorInfo();
        addAndInitSensorEnumInfo_Locked(halSensorDev, ret_count, mapToSensorType(pSensorInfo->GetType()), pSensorInfo->getDrvMacroName());//ToDo: modify argument

        //
        ret_count++;
    }

    if  ( (iSensorsList & SENSOR_DEV_SUB) == SENSOR_DEV_SUB )
    {
        halSensorDev = SENSOR_DEV_SUB;
        pSensorInfo = pSensorDrv->getSubSensorInfo();
        addAndInitSensorEnumInfo_Locked(halSensorDev, ret_count, mapToSensorType(pSensorInfo->GetType()), pSensorInfo->getDrvMacroName());//ToDo: modify argument

        //
        ret_count++;
    }
    //
    if  ( (iSensorsList & SENSOR_DEV_MAIN_2) == SENSOR_DEV_MAIN_2 )
    {
        halSensorDev = SENSOR_DEV_MAIN_2;
        pSensorInfo = pSensorDrv->getMain2SensorInfo();
        addAndInitSensorEnumInfo_Locked(halSensorDev, ret_count, mapToSensorType(pSensorInfo->GetType()), pSensorInfo->getDrvMacroName());//ToDo: modify argument


        //
        ret_count++;
    }

    pSeninfDrv->setMclk1(0, 1, 1, 0, 1, 0, 0);
    pSeninfDrv->setMclk2(0, 1, 1, 0, 1, 0, 0);
    //pSeninfDrv->setMclk3(0, 1, 1, 0, 1, 0, 0); /* No main2 */

    ret = pSeninfDrv->uninit();
    if(ret < 0) {
        MY_LOGE("pSeninfDrv->uninit() fail");
                return 0;
    }
    pSeninfDrv->destroyInstance();

    mEnumSensorCount = ret_count;
    return  ret_count;
}


/******************************************************************************
 *
 ******************************************************************************/
HalSensorList::EnumInfo const*
HalSensorList::
addAndInitSensorEnumInfo_Locked(
    MUINT           eSensorDev,
    MUINT           uSensorIndex,
    MUINT           eSensorType,
    char const*     szSensorDrvName
)
{
    mEnumSensorList.push_back(EnumInfo());
    //
    EnumInfo& rEnumInfo = mEnumSensorList.editTop();
    //
    rEnumInfo.meSensorDev = eSensorDev;
    rEnumInfo.muSensorIndex = uSensorIndex;
    rEnumInfo.meSensorType = eSensorType;
    rEnumInfo.ms8SensorDrvName = szSensorDrvName;
    buildStaticInfo(
        Info(uSensorIndex, eSensorDev, eSensorType, szSensorDrvName, sensorStaticInfo[uSensorIndex].facingDirection),
        rEnumInfo.mMetadata);
    //
    return  &rEnumInfo;
}

