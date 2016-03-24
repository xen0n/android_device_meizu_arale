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
#if (PLATFORM_SDK_VERSION >= 21)
#include "VendorTag/MTKVendorTags.h"
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
CamDeviceManagerBase::
~CamDeviceManagerBase()
{
    if  ( mpLibPlatform )
    {
        ::dlclose(mpLibPlatform);
        mpLibPlatform = NULL;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
CamDeviceManagerBase::
CamDeviceManagerBase()
    : ICamDeviceManager()
    , mRWLock()
    , mpLibPlatform(NULL)
    , mpModuleCallbacks(NULL)
    , mi4DeviceNum(0)
    , mEnumMap()
    , mOpenMap()
    , mVendorTagOps()
{
    MY_LOGD("%p", this);
    ::memset(&mVendorTagOps, 0, sizeof(mVendorTagOps));
#if (PLATFORM_SDK_VERSION >= 21)
    getMTKVendorTagsSingleton()->getVendorTagOps(&mVendorTagOps);
    //
    char const szModulePath[] = "libcamera_metadata.so";
    char const szEntrySymbol[] = "set_camera_metadata_vendor_ops";
    status_t res = NO_INIT;
    void* pfnEntry = NULL;
    void* pLib = ::dlopen(szModulePath, RTLD_NOW);
    if  ( ! pLib )
    {
        char const *err_str = ::dlerror();
        MY_LOGE("dlopen: %s error=%s", szModulePath, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    pfnEntry = ::dlsym(pLib, szEntrySymbol);
    if  ( ! pfnEntry )
    {
        char const *err_str = ::dlerror();
        MY_LOGE("dlsym: %s error=%s", szEntrySymbol, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    typedef int (*set_camera_metadata_vendor_ops)(const vendor_tag_ops_t *query_ops);
    res = reinterpret_cast<set_camera_metadata_vendor_ops>(pfnEntry)(&mVendorTagOps);

lbExit:
    if  ( pLib )
    {
        ::dlclose(pLib);
        pLib = NULL;
    }

    MY_LOGD("%p:%s(%p) return res:%s(%d) in %s", pfnEntry, szEntrySymbol, &mVendorTagOps, strerror(-res), res, szModulePath);
    MY_LOGE_IF(OK!=res, "static vendor metadata may fail");
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerBase::
open(
    hw_device_t** device,
    hw_module_t const* module,
    char const* name,
    uint32_t device_version
)
{
    int32_t const i4OpenId = (name != NULL) ? ::atoi(name) : -1;
    if  ( 0 == device_version ) {
        camera_info info;
        status_t status;
        if  ( OK != (status = getDeviceInfo(i4OpenId, info)) ) {
            return status;
        }
        device_version = info.device_version;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    return  openDeviceLocked(device, module, i4OpenId, device_version);
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerBase::
close(ICamDevice* pDevice)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    return  closeDeviceLocked(pDevice);
}


/******************************************************************************
 *
 ******************************************************************************/
int32_t
CamDeviceManagerBase::
getNumberOfDevices()
{
    RWLock::AutoWLock _l(mRWLock);
    //
    if  ( 0 != mi4DeviceNum )
    {
        MY_LOGD("#devices:%d", mi4DeviceNum);
    }
    else
    {
        Utils::CamProfile _profile(__FUNCTION__, "CamDeviceManagerBase");
        mi4DeviceNum = enumDeviceLocked();
        _profile.print("");
    }
    //
    return  mi4DeviceNum;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerBase::
getDeviceInfo(int const deviceId, camera_info& rInfo)
{
    RWLock::AutoRLock _l(mRWLock);
    //
    sp<EnumInfo> pEnumInfo = mEnumMap.valueFor(deviceId);
    if  ( pEnumInfo == 0 )
    {
        MY_LOGE("Bad deviceId:%d", deviceId);
        return -EINVAL;
    }
    //
    rInfo.device_version= pEnumInfo->uDeviceVersion;
    rInfo.facing        = pEnumInfo->iFacing;
    rInfo.orientation   = pEnumInfo->iWantedOrientation;
    rInfo.static_camera_characteristics = pEnumInfo->pMetadata;
    //
    rInfo.resource_cost = 0;
    rInfo.conflicting_devices = 0;
    rInfo.conflicting_devices_length = 0;
    //
    MY_LOGD("deviceId:%d device_version:0x%x facing:%d orientation:%d", deviceId, rInfo.device_version, rInfo.facing, rInfo.orientation);
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerBase::
setCallbacks(camera_module_callbacks_t const* callbacks)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    mpModuleCallbacks = callbacks;
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
void
CamDeviceManagerBase::
getVendorTagOps(
    vendor_tag_ops_t* ops
)
{
    MY_LOGD("ops:%p", ops);
    *ops = mVendorTagOps;
}

/******************************************************************************
 *
 ******************************************************************************/
status_t
CamDeviceManagerBase::
setTorchMode(int const deviceId, bool enabled)
{
    RWLock::AutoRLock _l(mRWLock);
    //
    sp<EnumInfo> pEnumInfo = mEnumMap.valueFor(deviceId);
    if  ( pEnumInfo == 0 )
    {
        MY_LOGE("Bad deviceId:%d", deviceId);
        return -EINVAL;
    }

    return  setTorchModeLocked(deviceId,enabled);
}

