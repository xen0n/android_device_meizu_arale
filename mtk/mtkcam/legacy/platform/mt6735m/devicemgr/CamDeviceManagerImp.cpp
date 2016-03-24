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

#define LOG_TAG "MtkCam/devicemgr"
//
#include "MyUtils.h"
#include "CamDeviceManagerImp.h"
#include <cutils/properties.h>
using namespace android;
using namespace NSCam;
//
#define OPTION__FORCE_ALL_DEVICE1   (0)
#define OPTION__NEED_DEVMETAINFO    (1)
#if OPTION__NEED_DEVMETAINFO
    #include <mtkcam/hal/sensor_hal.h>
    #include <mtkcam/v1/camutils/CamInfo.h>
    using namespace MtkCamUtils;
#endif
//
#include <mtkcam/hal/IHalFlash.h>
//
/******************************************************************************
 *
 ******************************************************************************/
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    #include <mtkcam/hal/IHalSensor.h>
#else
    #warning "[Warn] Not support Sensor Hal"
#endif
//
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
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
namespace
{
    CamDeviceManagerImp gCamDeviceManager;
}   //namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
ICamDeviceManager*
getCamDeviceManager()
{
    return  &gCamDeviceManager;
}
}


/******************************************************************************
 *
 ******************************************************************************/
CamDeviceManagerImp::
CamDeviceManagerImp()
    : CamDeviceManagerBase()
{
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerImp::
validateOpenLocked(int32_t i4OpenId, uint32_t device_version) const
{
    status_t status = OK;
    //
    status = CamDeviceManagerBase::validateOpenLocked(i4OpenId, device_version);
    if  ( OK != status )
    {
        return  status;
    }
    //
    //
    //[Now] try to open camera1 [Previous] no camera open [Result] success
    //[Now] try to open camera1 [Previous] camera1 opened [Result] success if PIP is supported
    //[Now] try to open camera1 [Previous] camera3 opened [Result] failure
    if  ( device_version == CAMERA_DEVICE_API_VERSION_1_0 ) {
        if  ( 0 != mOpenMap.size() )
        {
            if  ( mOpenMap.valueAt(0)->uDeviceVersion >= CAMERA_DEVICE_API_VERSION_3_0 ) {
                MY_LOGE("[Now] fail to open (deviceId:%d version:0x%x) => failure", i4OpenId, device_version);
                MY_LOGE("[Previous] (deviceId:%d version:0x%x) mOpenMap.size:%zu", mOpenMap.keyAt(0), mOpenMap.valueAt(0)->uDeviceVersion, mOpenMap.size());
                return -EUSERS;
            }
            //
#if 1
            if  ( mOpenMap.valueAt(0)->uDeviceVersion == CAMERA_DEVICE_API_VERSION_1_0 ) {
                MY_LOGE("[Now] fail to open (deviceId:%d version:0x%x) => failure", i4OpenId, device_version);
                MY_LOGE("[Previous] (deviceId:%d version:0x%x) mOpenMap.size:%zu", mOpenMap.keyAt(0), mOpenMap.valueAt(0)->uDeviceVersion, mOpenMap.size());
                return -EUSERS;
            }
#endif
        }
    }
    //
    //[Now] try to open camera3 [Previous] no camera open [Result] success
    else
    if  ( device_version >= CAMERA_DEVICE_API_VERSION_3_0 ) {
        if  ( 0 != mOpenMap.size() )
        {
            MY_LOGE("[Now] fail to open (deviceId:%d version:0x%x) => failure", i4OpenId, device_version);
            MY_LOGE("[Previous] (deviceId:%d version:0x%x) mOpenMap.size:%zu", mOpenMap.keyAt(0), mOpenMap.valueAt(0)->uDeviceVersion, mOpenMap.size());
            /*
             * -EUSERS:     The maximal number of camera devices that can be
             *              opened concurrently were opened already, either by
             *              this method or common.methods->open method.
             */
            return -EUSERS;
        }
    }
    else {
        /*
         * -EOPNOTSUPP: The requested HAL version is not supported by this method.
         */
        MY_LOGE("Unsupported version:0x%x", device_version);
        return -EOPNOTSUPP;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
int32_t
CamDeviceManagerImp::
enumDeviceLocked()
{
    status_t status = OK;
    int32_t i4DeviceNum = 0;
    //
#if '1'==MTKCAM_HAVE_METADATA
    NSMetadataProviderManager::clear();
#endif
#if OPTION__NEED_DEVMETAINFO
    DevMetaInfo::clear();
#endif
    mEnumMap.clear();
//------------------------------------------------------------------------------
#if '1'==MTKCAM_HAVE_SENSOR_HAL
    //
    IHalSensorList*const pHalSensorList = IHalSensorList::get();
    size_t const sensorNum = pHalSensorList->searchSensors();
#if 0
    SensorHal::createInstance()->searchSensor();;
#endif
    CAM_LOGD("pHalSensorList:%p searchSensors:%zu queryNumberOfSensors:%d", pHalSensorList, sensorNum, pHalSensorList->queryNumberOfSensors());
    mEnumMap.setCapacity(sensorNum+1);
    for (size_t i = 0; i < sensorNum; i++)
    {
        int32_t const deviceId = i;
        //
        sp<EnumInfo> pInfo = new EnumInfo;
        mEnumMap.add(deviceId, pInfo);
        //
#if '1'==MTKCAM_HAVE_METADATA
        sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(deviceId);
        NSMetadataProviderManager::add(deviceId, pMetadataProvider.get());
        MY_LOGD("[0x%02x] IMetadataProvider:%p sensor:%s", deviceId, pMetadataProvider.get(), pHalSensorList->queryDriverName(i));
        //
#if OPTION__FORCE_ALL_DEVICE1
        pInfo->uDeviceVersion       = CAMERA_DEVICE_API_VERSION_1_0;
#else
        {
            char value[PROPERTY_VALUE_MAX];
            ::property_get("debug.camera.force_device", value, "0");
            switch( atoi(value) )
            {
                case 1:
                    MY_LOGD("debug.camera.force_device = %s", value);
                    pInfo->uDeviceVersion       = CAMERA_DEVICE_API_VERSION_1_0;
                    break;
                #if (PLATFORM_SDK_VERSION >= 21)
                case 3:
                    MY_LOGD("debug.camera.force_device = %s", value);
                    pInfo->uDeviceVersion       = CAMERA_DEVICE_API_VERSION_3_2;
                    break;
                #endif
                default:
                    pInfo->uDeviceVersion       = pMetadataProvider->getDeviceVersion();
                    break;
            };
        }
#endif
        pInfo->pMetadata            = pMetadataProvider->getStaticCharacteristics();
        pInfo->iFacing              = (pMetadataProvider->getDeviceFacing() == MTK_LENS_FACING_FRONT)
                                        ? CAMERA_FACING_FRONT
                                        : CAMERA_FACING_BACK
                                        ;
        pInfo->iWantedOrientation   = pMetadataProvider->getDeviceWantedOrientation();
        pInfo->iSetupOrientation    = pMetadataProvider->getDeviceSetupOrientation();
        pInfo->iHasFlashLight       = pMetadataProvider->getDeviceHasFlashLight();
#endif

#if OPTION__NEED_DEVMETAINFO
        camera_info camInfo;
        camInfo.facing = pInfo->iFacing;
        camInfo.orientation = pInfo->iWantedOrientation;
        DevMetaInfo::add(i, camInfo, pInfo->iSetupOrientation, eDevId_ImgSensor, pHalSensorList->querySensorDevIdx(i));
#endif
        //
        i4DeviceNum++;
    }
    //
    // mATV
    {
        int32_t const deviceId = 0xFF;
        //
        sp<EnumInfo> pInfo = new EnumInfo;
        pInfo->uDeviceVersion       = CAMERA_DEVICE_API_VERSION_1_0;
        pInfo->pMetadata            = NULL;
        pInfo->iFacing              = 0;
        pInfo->iWantedOrientation   = 0;
        pInfo->iSetupOrientation    = 0;
        mEnumMap.add(deviceId, pInfo);
        //
#if OPTION__NEED_DEVMETAINFO
        camera_info camInfo;
        camInfo.facing = 0;
        camInfo.orientation = 0;
        DevMetaInfo::add(deviceId, camInfo, camInfo.orientation, eDevId_AtvSensor, SENSOR_DEV_ATV);
#endif
        //
//        i4DeviceNum++;  // don't count it in
    }
    //
    //
    MY_LOGI("i4DeviceNum=%d", i4DeviceNum);
    for (size_t i = 0; i < mEnumMap.size(); i++)
    {
        int32_t const deviceId = mEnumMap.keyAt(i);
        sp<EnumInfo> pInfo = mEnumMap.valueAt(i);
        uint32_t const uDeviceVersion   = pInfo->uDeviceVersion;
        camera_metadata const*pMetadata = pInfo->pMetadata;
        int32_t const iFacing           = pInfo->iFacing;
        int32_t const iWantedOrientation= pInfo->iWantedOrientation;
        int32_t const iSetupOrientation = pInfo->iSetupOrientation;
        MY_LOGI(
            "[0x%02x] DeviceVersion:0x%x metadata:%08p facing:%d"
            " orientation(wanted/setup)=(%d/%d)",
            deviceId, pInfo->uDeviceVersion, pInfo->pMetadata, pInfo->iFacing,
            pInfo->iWantedOrientation, pInfo->iSetupOrientation

        );
    }
//------------------------------------------------------------------------------
#else   //#if '1'==MTKCAM_HAVE_SENSOR_HAL
//------------------------------------------------------------------------------

    #warning "[WARN] Simulation for CamDeviceManagerImp::enumDeviceLocked()"

    {
        int32_t const deviceId = 0;
        //
        sp<EnumInfo> pInfo = new EnumInfo;
        mEnumMap.add(deviceId, pInfo);
        //
#if '1'==MTKCAM_HAVE_METADATA
        sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(deviceId);
        if  ( pMetadataProvider == 0 )
        {
            MY_LOGE("[%d] IMetadataProvider::create", deviceId);
        }
        NSMetadataProviderManager::add(deviceId, pMetadataProvider.get());
        //
        pInfo->uDeviceVersion       = pMetadataProvider->getDeviceVersion();
        pInfo->pMetadata            = pMetadataProvider->getStaticCharacteristics();
        pInfo->iFacing              = pMetadataProvider->getDeviceFacing();
        pInfo->iWantedOrientation   = pMetadataProvider->getDeviceWantedOrientation();
        pInfo->iSetupOrientation    = pMetadataProvider->getDeviceSetupOrientation();
#endif
        //
        i4DeviceNum++;
    }
    //
    // mATV
    {
        int32_t const deviceId = 0xFF;
        //
        sp<EnumInfo> pInfo = new EnumInfo;
        mEnumMap.add(deviceId, pInfo);
        pInfo->uDeviceVersion       = CAMERA_DEVICE_API_VERSION_1_0;
        pInfo->pMetadata            = NULL;
        pInfo->iFacing              = 0;
        pInfo->iWantedOrientation   = 0;
        pInfo->iSetupOrientation    = 0;
    }
#endif  //#if '1'==MTKCAM_HAVE_SENSOR_HAL
//------------------------------------------------------------------------------
    //
    return  i4DeviceNum;
}


status_t
CamDeviceManagerImp::
setTorchModeLocked(int const deviceId, bool enabled, bool notifyEvent = true)
{
    sp<EnumInfo> pInfo = mEnumMap.valueAt(deviceId);
    if (pInfo->iHasFlashLight == 0) {
         MY_LOGW("setTorchAvailableLocked Failed[No Flash]: deviceId:%d",deviceId);
        return -EINVAL;
    }

    IHalFlash*const pHalFlash = IHalFlash::getInstance();
    bool flashStatus = pHalFlash->getTorchStatus(deviceId) == 1;
    MY_LOGD("setTorchModeLocked: deviceId:%d, flashStatus:%d, enable:%d",deviceId ,flashStatus, enabled);
    if (flashStatus != enabled) {
        if (pHalFlash->setTorchOnOff(deviceId, enabled) == OK) {
            if (notifyEvent) {
                String8 cameraId = String8::format("%d", deviceId);
                mpModuleCallbacks->torch_mode_status_change(mpModuleCallbacks, cameraId.string(),
                                                            enabled ? TORCH_MODE_STATUS_AVAILABLE_ON : TORCH_MODE_STATUS_AVAILABLE_OFF);
            }
        } else {
            MY_LOGW("SetTorchModeLocked Failed: deviceId:%d, enable:%d",deviceId, enabled);
            return -EINVAL;
        }
    }
    return OK;
}


status_t
CamDeviceManagerImp::
setTorchAvailableLocked(int const deviceId, bool available)
{
    sp<EnumInfo> pInfo = mEnumMap.valueAt(deviceId);
    if(pInfo->iHasFlashLight == 0){
        MY_LOGW("setTorchAvailableLocked Failed[No Flash]: deviceId:%d",deviceId);
        return -EINVAL;
    }
    MY_LOGD("setTorchAvailableLocked: deviceId:%d, available:%d",deviceId ,available);
    // notify camera service for locking/unlocking touch mode
    String8 cameraId = String8::format("%d", deviceId);
    if(mpModuleCallbacks==NULL || mpModuleCallbacks->torch_mode_status_change==NULL){
        MY_LOGW("setTorchAvailableLocked callbacks null");
        return -ENOSYS;
    }
    mpModuleCallbacks->torch_mode_status_change(mpModuleCallbacks, cameraId.string(),
                                                available ? TORCH_MODE_STATUS_AVAILABLE_OFF : TORCH_MODE_STATUS_NOT_AVAILABLE);
    return OK;
}
