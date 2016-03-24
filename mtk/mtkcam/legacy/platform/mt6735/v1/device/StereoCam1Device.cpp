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

#define LOG_TAG "MtkCam/Cam1Device"
//
#include "MyUtils.h"
#include "StereoCam1Device.h"
//
using namespace android;
//
#include <mtkcam/utils/common.h>
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam;
//
#include <sys/prctl.h>


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s:%d)[%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s:%d)[%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s:%d)[%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s:%d)[%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s:%d)[%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s:%d)[%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s:%d)[%s] " fmt, ::gettid(), getDevName(), getOpenId(), __FUNCTION__, ##arg)
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
extern "C"
NSCam::Cam1Device*
createCam1Device_MtkStereo(
    String8 const&          rDevName,
    int32_t const           i4OpenId
)
{
    return new StereoCam1Device(rDevName, i4OpenId);
}


/******************************************************************************
 *
 ******************************************************************************/
StereoCam1Device::
StereoCam1Device(
    String8 const&          rDevName,
    int32_t const           i4OpenId
)
    : Cam1DeviceBase(rDevName, i4OpenId)
    //
    , mpHalSensor(NULL)
    //
#if '1'==MTKCAM_HAVE_3A_HAL
    , mpHal3a_Main(NULL)
    , mpHal3a_Main2(NULL)
    , mpSync3AMgr(NULL)
#endif
    , mbThreadRunning(MFALSE)
    //
    , mSensorId_Main(-1)
    , mSensorId_Main2(-1)
{
}


/******************************************************************************
 *
 ******************************************************************************/
StereoCam1Device::
~StereoCam1Device()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
StereoCam1Device::
onInit()
{
    MY_LOGD("+");
    {
        CAM_TRACE_NAME("init(profile)");
        initPlatformProfile();
    }
    AutoCPTLog cptlog(Event_Hal_DefaultCamDevice_init);
    Utils::CamProfile  profile(__FUNCTION__, "StereoCam1Device");
    //
    bool    ret = false;
    int     err = 0, i4DeviceNum = 0;
    //
    String8 const s8MainIdKey("MTK_SENSOR_DEV_MAIN");
    String8 const s8Main2IdKey("MTK_SENSOR_DEV_MAIN_2");
    Utils::Property::tryGet(s8MainIdKey, mSensorId_Main);
    Utils::Property::tryGet(s8Main2IdKey, mSensorId_Main2);
    if  ( -1 == getOpenId_Main() || -1 == getOpenId_Main2() ) {
        MY_LOGE("no dual main sensor");
        goto lbExit;
    }
    //
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_CAMDRV
    {
        CAM_TRACE_NAME("init(resource)");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource +");
        IResManager* pResManager = IResManager::getInstance();
        //
        if  ( pResManager != NULL )
        {
            if(!(pResManager->open(USER_NAME)))
            {
                MY_LOGE("pResManager->open fail");
                goto lbExit;
            }
        }
        //
        profile.print("Resource -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource -");
    }
#endif  //MTKCAM_HAVE_CAMDRV
    //--------------------------------------------------------------------------
    {
        CAM_TRACE_NAME("init(sensor)");
        //  (0) power on sensor
        if( pthread_create(&mThreadHandle, NULL, doThreadInit, this) != 0 )
        {
            MY_LOGE("pthread create failed");
            goto lbExit;
        }
        mbThreadRunning = MTRUE;
        //
        // workaround: yuv sensor, 3A depends on sensor power-on
        if( NSCam::IHalSensorList::get()->queryType( getOpenId_Main() ) == NSCam::NSSensorType::eYUV ||
            NSCam::IHalSensorList::get()->queryType( getOpenId_Main2() ) == NSCam::NSSensorType::eYUV )
        {
            if( !waitThreadInitDone() )
            {
                MY_LOGE("init in thread failed");
                goto lbExit;
            }
        }
    }
    //--------------------------------------------------------------------------
        //  (1) Open 3A
#if '1'==MTKCAM_HAVE_3A_HAL
    {
        CAM_TRACE_NAME("init(3A)");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal +");

#ifdef MTK_NATIVE_3D_SUPPORT
        mpSync3AMgr = NS3A::ISync3AMgr::getInstance();
        if  ( ! mpSync3AMgr ) {
            MY_LOGE("ISync3AMgr::getInstance() fail");
            goto lbExit;
        }
        if  ( ! mpSync3AMgr->init(0, getOpenId_Main(), getOpenId_Main2()) ) {
            MY_LOGE("mpSync3AMgr->init fail");
            goto lbExit;
        }
#endif

        mpHal3a_Main = NS3A::IHal3A::createInstance(
                NS3A::IHal3A::E_Camera_1,
                getOpenId_Main(),
                LOG_TAG);
        if  ( ! mpHal3a_Main ) {
            MY_LOGE("IHal3A::createInstance() fail");
            goto lbExit;
        }
        mpHal3a_Main2 = NS3A::IHal3A::createInstance(
                NS3A::IHal3A::E_Camera_1,
                getOpenId_Main2(),
                LOG_TAG);
        if  ( ! mpHal3a_Main2 ) {
            MY_LOGE("IHal3A::createInstance() fail: main2");
            goto lbExit;
        }
        profile.print("3A Hal -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal -");
    }
#endif  //MTKCAM_HAVE_3A_HAL
    //--------------------------------------------------------------------------
    {
        CAM_TRACE_NAME("init(base)");
        //  (2) Init Base.
        if  ( ! Cam1DeviceBase::onInit() )
        {
            goto lbExit;
        }
    }
    //
    //--------------------------------------------------------------------------
    //
    ret = true;
lbExit:
    profile.print("");
    MY_LOGD("- ret(%d) sensorId(%d,%d)", ret, getOpenId_Main(), getOpenId_Main2());
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
StereoCam1Device::
onUninit()
{
    MY_LOGD("+");
    AutoCPTLog cptlog(Event_Hal_DefaultCamDevice_uninit);
    Utils::CamProfile  profile(__FUNCTION__, "StereoCam1Device");
    //
    if( !waitThreadInitDone() )
    {
        MY_LOGE("init in thread failed");
    }
    //--------------------------------------------------------------------------
    //  (1) Uninit Base
    {
        CAM_TRACE_NAME("uninit(base)");
        Cam1DeviceBase::onUninit();
        profile.print("Cam1DeviceBase::onUninit() -");
    }
    //--------------------------------------------------------------------------
    //  (2) Close 3A
#if '1'==MTKCAM_HAVE_3A_HAL
    {
        CAM_TRACE_NAME("uninit(3A)");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal +");
#ifdef MTK_NATIVE_3D_SUPPORT
        if  ( mpSync3AMgr )
        {
            mpSync3AMgr->uninit();
            mpSync3AMgr = NULL;
        }
#endif
        if  ( mpHal3a_Main )
        {
            mpHal3a_Main->destroyInstance(LOG_TAG);
            mpHal3a_Main = NULL;
        }
        if  ( mpHal3a_Main2 )
        {
            mpHal3a_Main2->destroyInstance(LOG_TAG);
            mpHal3a_Main2 = NULL;
        }
        profile.print("3A Hal -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "3A Hal -");
    }
#endif  //MTKCAM_HAVE_3A_HAL
    //--------------------------------------------------------------------------
    //  (4) Close Sensor
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    {
        CAM_TRACE_NAME("uninit(sensor)");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal +");
        //
        if(mpHalSensor)
        {
            MUINT pIndex[2] = { (MUINT)getOpenId_Main(), (MUINT)getOpenId_Main2() };
            mpHalSensor->powerOff(USER_NAME, 2, pIndex);
            mpHalSensor->destroyInstance(USER_NAME);
            mpHalSensor = NULL;
        }
        MY_LOGD("SensorHal::destroyInstance()");
        profile.print("Sensor Hal -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal -");
    }
#endif  //MTKCAM_HAVE_SENSOR_HAL
    //--------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_CAMDRV
    {
        CAM_TRACE_NAME("uninit(resource)");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource +");
    IResManager* pResManager = IResManager::getInstance();
    //
    if  ( pResManager != NULL )
    {
            if(!(pResManager->close(USER_NAME)))
        {
            MY_LOGE("pResManager->close fail");
        }
    }
        profile.print("Resource -");
        CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Resource -");
    }
#endif  //MTKCAM_HAVE_CAMDRV
    //--------------------------------------------------------------------------
    //
    profile.print("");
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 * [Template method] Called by startPreview().
 ******************************************************************************/
bool
StereoCam1Device::
onStartPreview()
{
    bool ret = false;
    Utils::CamManager* pCamMgr = Utils::CamManager::getInstance();
    //
    //  (0) wait for thread
    if( !waitThreadInitDone() )
    {
        MY_LOGE("init in thread failed");
        goto lbExit;
    }
    //
    //  (1) Check Permission.
    if ( ! pCamMgr->getPermission() )
    {
        MY_LOGE("Cannot start preview ... Permission denied");
        goto lbExit;
    }

    //  (3) Initialize Camera Adapter.
    if  ( ! initCameraAdapter() )
    {
        MY_LOGE("NULL Camera Adapter");
        goto lbExit;
    }
    //
    ret = true;
lbExit:
    return ret;
}


/******************************************************************************
 *  [Template method] Called by stopPreview().
 ******************************************************************************/
void
StereoCam1Device::
onStopPreview()
{
    if  ( mpCamAdapter != 0 )
    {
        mpCamAdapter->cancelPicture();
        mpCamAdapter->uninit();
        mpCamAdapter.clear();
    }
}


/******************************************************************************
 * Create a thread to hide some initial steps to speed up launch time
 ******************************************************************************/
bool
StereoCam1Device::
powerOnSensor()
{
    MY_LOGD("+");
    bool    ret = false;
    //  (1) Open Sensor
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    CAM_TRACE_CALL();
    Utils::CamProfile  profile(__FUNCTION__, "DefaultCam1Device");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal +");
    //
    NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
    MUINT pIndex[2] = { (MUINT)getOpenId_Main(), (MUINT)getOpenId_Main2() };
    if(!pHalSensorList)
    {
        MY_LOGE("pHalSensorList == NULL");
        goto lbExit;
    }
    //
    mpHalSensor = pHalSensorList->createSensor(
                                        USER_NAME,
                                        2,
                                        pIndex);
    if(mpHalSensor == NULL)
    {
       MY_LOGE("mpHalSensor is NULL");
       goto lbExit;
    }
    //
    if( !mpHalSensor->powerOn(USER_NAME, 2, pIndex) )
    {
        MY_LOGE("sensor power on failed: %d, %d", pIndex[0], pIndex[1]);
        goto lbExit;
    }
    //
    profile.print("Sensor Hal -");
    CPTLogStr(Event_Hal_DefaultCamDevice_init, CPTFlagSeparator, "Sensor Hal -");
#endif  //MTKCAM_HAVE_SENSOR_HAL

    ret = true;
lbExit:
    MY_LOGD("-");
    return ret;
}


/******************************************************************************
 * the init function to be called in the thread
 ******************************************************************************/
void*
StereoCam1Device::
doThreadInit(void* arg)
{
    ::prctl(PR_SET_NAME,"initCamdevice", 0, 0, 0);
    StereoCam1Device* pSelf = reinterpret_cast<StereoCam1Device*>(arg);
    pSelf->mRet = pSelf->powerOnSensor();
    pthread_exit(NULL);
    return NULL;
}


/******************************************************************************
 * Wait for initializations by thread are done.
 ******************************************************************************/
bool
StereoCam1Device::
waitThreadInitDone()
{
    bool ret = false;
    if( mbThreadRunning )
    {
        MY_LOGD("wait init done +");
        int s = pthread_join(mThreadHandle, NULL);
        MY_LOGD("wait init done -");
        mbThreadRunning = MFALSE;
        if( s != 0 )
        {
            MY_LOGE("pthread join error: %d", s);
            goto lbExit;
        }

        if( !mRet )
        {
            MY_LOGE("init in thread failed");
            goto lbExit;
        }
    }

    ret = true;
lbExit:
    return ret;
}


/******************************************************************************
 *  Set the camera parameters. This returns BAD_VALUE if any parameter is
 *  invalid or not supported.
 ******************************************************************************/
status_t
StereoCam1Device::
setParameters(const char* params)
{
    CAM_TRACE_CALL();
    status_t status = OK;
    //
    //  (1) Update params to mpParamsMgr.
    status = mpParamsMgr->setParameters(String8(params));
    if  ( OK != status ) {
        goto lbExit;
    }

    //  Here (1) succeeded.
    //  (2) If CamAdapter exists, apply mpParamsMgr to CamAdapter;
    //      otherwise it will be applied when CamAdapter is created.
    {
        sp<ICamAdapter> pCamAdapter = mpCamAdapter;
        if  ( pCamAdapter != 0 ) {
            status = pCamAdapter->setParameters();
        }
    }

lbExit:
    return  status;
}

