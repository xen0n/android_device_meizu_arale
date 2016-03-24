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

// get_boot_mode
#include <sys/stat.h>
#include <fcntl.h>
//
#include <binder/IServiceManager.h>
#include <camera/MtkCameraParameters.h>
//
#if (MTKCAM_BASIC_PACKAGE != 1)
#include <mmsdk/IMMSdkService.h>
#endif
//
#include <cutils/properties.h>
#include "MyUtils.h"
//
#if (MTKCAM_BASIC_PACKAGE != 1)
using namespace NSMMSdk;
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
#if (MTKCAM_BASIC_PACKAGE != 1)
sp<IMMSdkService>
getMMSdkService()
{
    sp<IServiceManager> sm = defaultServiceManager();
    if (sm == 0)
    {
        MY_LOGE("Can not get the service mananger");
    }

    // use checkService to avoid blocking if mmsdk service is not up yet
    sp<IBinder> binder = sm->checkService(String16("media.mmsdk"));
    if (binder == 0)
    {
        MY_LOGE("Can not get mmsdk service");
        return 0;
    }
    //
    sp<IMMSdkService> service = interface_cast<IMMSdkService>(binder);
    if (service == 0)
    {
        MY_LOGE("Null mmsdk service");
        return 0;
    }
    return service;
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
 extern "C"
{
int getBootMode(void)
{
  int fd;
  size_t s;
  char boot_mode[4] = {'0'};

  fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDWR);
  if (fd < 0)
  {
    MY_LOGW("fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_mode");
    return 0;
  }

  s = read(fd, (void *)&boot_mode, sizeof(boot_mode) - 1);
  close(fd);

  if(s <= 0)
  {
        MY_LOGW("could not read boot mode sys file\n");
    return 0;
  }

  boot_mode[s] = '\0';
  return atoi(boot_mode);
}
}


/******************************************************************************
 *
 ******************************************************************************/
CamDeviceManagerBase::
OpenInfo::
~OpenInfo()
{
}


/******************************************************************************
 *
 ******************************************************************************/
CamDeviceManagerBase::
OpenInfo::
OpenInfo()
    : RefBase()
    , pDevice(0)
    , uDeviceVersion(0)
    , i8OpenTimestamp(0)
{
}


/******************************************************************************
 *
 ******************************************************************************/
static
String8 const
queryClientAppMode()
{
/*
    Before opening camera, client must call
    Camera::setProperty(
        String8(MtkCameraParameters::PROPERTY_KEY_CLIENT_APPMODE),
        String8(MtkCameraParameters::APP_MODE_NAME_MTK_xxx)
    ),
    where MtkCameraParameters::APP_MODE_NAME_MTK_xxx = one of the following:
        MtkCameraParameters::APP_MODE_NAME_DEFAULT
        MtkCameraParameters::APP_MODE_NAME_MTK_ENG
        MtkCameraParameters::APP_MODE_NAME_MTK_ATV
        MtkCameraParameters::APP_MODE_NAME_MTK_S3D
        MtkCameraParameters::APP_MODE_NAME_MTK_VT
*/
    String8 const s8ClientAppModeKey(MtkCameraParameters::PROPERTY_KEY_CLIENT_APPMODE);
    String8       s8ClientAppModeVal(MtkCameraParameters::APP_MODE_NAME_DEFAULT);
    //
    //  (1) get Client's property.
    Utils::Property::tryGet(s8ClientAppModeKey, s8ClientAppModeVal);
    if  ( s8ClientAppModeVal.isEmpty() ) {
        s8ClientAppModeVal = MtkCameraParameters::APP_MODE_NAME_DEFAULT;
    }
    //
    //  (2) reset Client's property.
    Utils::Property::set(s8ClientAppModeKey, String8::empty());
    //
    //
    return  s8ClientAppModeVal;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerBase::
detachDeviceLocked(android::sp<ICamDevice> pDevice)
{
    sp<OpenInfo> pOpenInfo;
    int32_t const openId = pDevice->getOpenId();
    //
    String8 const s8ClientAppMode = queryClientAppMode();
    //
    ssize_t const index = mOpenMap.indexOfKey(openId);
    pOpenInfo = mOpenMap.valueAt(index);
    if  (
            index < 0
        ||  pOpenInfo == 0
        ||  pOpenInfo->pDevice != pDevice
        )
    {
        MY_LOGE("device %d: not found!!! mOpenMap.size:%d index:%d pOpenInfo:%p", openId, mOpenMap.size(), index, pOpenInfo.get());
        MY_LOGE_IF(pOpenInfo != 0, "device %p %p", pOpenInfo->pDevice.get(), pDevice.get());
        return  NAME_NOT_FOUND;
    }
    #if (MTKCAM_BASIC_PACKAGE != 1)
    // un-register device to MMSDK service
    if (pDevice != 0 &&  pOpenInfo->uDeviceVersion == CAMERA_DEVICE_API_VERSION_1_0)   //device init success, register itself to mmsdk service
    {
         // service manager only start at normal mode
         if (0 == getBootMode())
         {
             sp<IMMSdkService> cs = getMMSdkService();
             if (cs != 0)
             {
                 cs->unRegisterCamera1Device(pDevice.get());
             }
         }
    }
    #endif
    //
    mOpenMap.removeItemsAt(index);
    MY_LOGD("device: %s %d", pDevice->getDevName(), pDevice->getOpenId());
    // Unlock torch when all cameras closed
    setTorchModeLocked(openId, false, false);
    setTorchAvailableLocked(openId, true);
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerBase::
attachDeviceLocked(android::sp<ICamDevice> pDevice, uint32_t device_version)
{
    sp<OpenInfo> pOpenInfo;
    int32_t const openId = pDevice->getOpenId();
    //
    //
    pOpenInfo = mOpenMap.valueFor(openId);
    if  ( pOpenInfo != 0 )
    {
        sp<ICamDevice> const pDev = pOpenInfo->pDevice;
        MY_LOGE(
            "Busy deviceId:%d; device:%p has already been opend with version:0x%x OpenTimestamp:%lld",
            openId, pDev.get(), pOpenInfo->uDeviceVersion, pOpenInfo->i8OpenTimestamp
        );
        MY_LOGE_IF(pDev != 0, "device: %s %d", pDev->getDevName(), pDev->getOpenId());

        return  ALREADY_EXISTS;
    }
    //
    //
    pOpenInfo = new OpenInfo;
    pOpenInfo->pDevice = pDevice;
    pOpenInfo->uDeviceVersion = device_version;
    pOpenInfo->i8OpenTimestamp = ::systemTime();
    //
    mOpenMap.add(openId, pOpenInfo);
    MY_LOGD(
        "device: %s %d version:0x%x OpenTimestamp:%lld",
        pDevice->getDevName(),
        pDevice->getOpenId(),
        pOpenInfo->uDeviceVersion,
        pOpenInfo->i8OpenTimestamp
    );
    // Lock torch when first camera opened
    setTorchModeLocked(openId, false, false);
    setTorchAvailableLocked(openId, false);
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerBase::
validateOpenLocked(int32_t i4OpenId, uint32_t device_version) const
{
    sp<EnumInfo> pEnumInfo = mEnumMap.valueFor(i4OpenId);
    if  ( pEnumInfo == 0 ) {
        MY_LOGE(
            "Bad OpenId:%d - version:0x%x mEnumMap.size:%zu DeviceNum:%d",
            i4OpenId, device_version, mEnumMap.size(), mi4DeviceNum
        );
        /*
         * -EINVAL:     The input arguments are invalid, i.e. the id is invalid,
         *              and/or the module is invalid.
         */
        return -EINVAL;
    }
    //
    sp<OpenInfo> pOpenInfo = mOpenMap.valueFor(i4OpenId);
    if  ( pOpenInfo != 0 ) {
        sp<ICamDevice> const pDev = pOpenInfo->pDevice;
        MY_LOGE(
            "Busy deviceId:%d; device:%p has already been opend with version:0x%x OpenTimestamp:%lld",
            i4OpenId, pDev.get(), pOpenInfo->uDeviceVersion, pOpenInfo->i8OpenTimestamp
        );
        MY_LOGE_IF(pDev != 0, "device: %s %d", pDev->getDevName(), pDev->getOpenId());
        /*
         * -EBUSY:      The camera device was already opened for this camera id
         *              (by using this method or common.methods->open method),
         *              regardless of the device HAL version it was opened as.
         */
        return -EBUSY;
    }
    //
    return  validateOpenLocked(i4OpenId);
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerBase::
closeDeviceLocked(android::sp<ICamDevice> pDevice)
{
    //  reset Client's property.
    String8 const s8ClientAppModeKey(MtkCameraParameters::PROPERTY_KEY_CLIENT_APPMODE);
    Utils::Property::set(s8ClientAppModeKey, String8::empty());

    return  detachDeviceLocked(pDevice);
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerBase::
openDeviceLocked(
    hw_device_t** device,
    hw_module_t const* module,
    int32_t const i4OpenId,
    uint32_t device_version
)
{
    status_t status = OK;
    sp<ICamDevice> pDevice = NULL;
    //
    String8 const s8ClientAppMode = queryClientAppMode();
    //
    MY_LOGD(
        "+ OpenId:%d with version 0x%x - mOpenMap.size:%zu mEnumMap.size:%zu",
        i4OpenId, device_version, mOpenMap.size(), mEnumMap.size()
    );
    //
    //  [1] check to see whether it's ready to open.
    if  ( OK != (status = validateOpenLocked(i4OpenId, device_version)) )
    {
        return  status;
    }
    //
    //  [2] get platform
    IPlatform*const pPlatform = getPlatform();
    if  ( ! pPlatform )
    {
        MY_LOGE("No Platform");
        return  -ENODEV;
    }
    //
    //  [3] create device based on device version.
    if  ( device_version == CAMERA_DEVICE_API_VERSION_1_0 )
    {
        int i4DebugOpenID = -1;
        // try get property from system property
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get( "debug.camera.open", value, "-1");
        i4DebugOpenID = atoi(value);
        if( i4DebugOpenID != -1 )
        {
            pDevice = pPlatform->createCam1Device(s8ClientAppMode.string(), i4DebugOpenID);
            MY_LOGD("force to open camera:%d", i4DebugOpenID);
        }
        else
        {
            // try get property from AP
            int i4DebugOpenID = -1;
            Utils::Property::tryGet(String8("debug.camera.open"), i4DebugOpenID);
            pDevice = pPlatform->createCam1Device(s8ClientAppMode.string(), i4DebugOpenID == -1 ? i4OpenId:i4DebugOpenID);
        }
        #if (MTKCAM_BASIC_PACKAGE != 1)
        //register device to MMSDK service
        if (pDevice != 0)   //device init success, register itself to mmsdk service
        {
             // service manager only start at normal mode
             if (0 == getBootMode())
             {
                 sp<IMMSdkService> cs = getMMSdkService();
                 if (cs != 0)
                 {
                     cs->registerCamera1Device(pDevice.get());
                 }
             }
        }
        #endif
    }
    else
    if  ( device_version >= CAMERA_DEVICE_API_VERSION_3_0 )
    {
        pDevice = pPlatform->createCam3Device(s8ClientAppMode.string(), i4OpenId);
    }
    else
    {
        MY_LOGE("Unsupported version:0x%x", device_version);
        return  -EOPNOTSUPP;
    }
    //
    if  ( pDevice == 0 )
    {
        MY_LOGE("device creation failure");
        return  -ENODEV;
    }
    //
    //  [4] open device successfully.
    {
        *device = const_cast<hw_device_t*>(pDevice->get_hw_device());
        //
        pDevice->set_hw_module(module);
        pDevice->set_module_callbacks(mpModuleCallbacks);
        pDevice->setDeviceManager(this);
        //
        attachDeviceLocked(pDevice, device_version);
    }
    //
    return  OK;
}

