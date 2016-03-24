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

#define LOG_TAG "MtkCam/Cam3Device"
//
#include "MyUtils.h"
#include <mtkcam/device/Cam3Device.h>
//
using namespace android;
using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
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
static
inline
Cam3Device*
getDevice(camera3_device const* device)
{
    return  Cam3Device::getDevice(device);
}


////////////////////////////////////////////////////////////////////////////////
//  Implementation of hw_device_t
////////////////////////////////////////////////////////////////////////////////
static
int
camera_close_device(hw_device_t* device)
{
    if  ( ! device )
    {
        return  -EINVAL;
    }
    //
    return  Cam3Device::getDevice(device)->i_closeDevice();
}


static
hw_device_t const
gHwDevice =
{
    /** tag must be initialized to HARDWARE_DEVICE_TAG */
    tag:        HARDWARE_DEVICE_TAG,
    /** version number for hw_device_t */
    version:    CAMERA_DEVICE_API_VERSION_3_2,
    /** reference to the module this device belongs to */
    module:     NULL,
    /** padding reserved for future use */
    reserved:   {0},
    /** Close this device */
    close:      camera_close_device,
};


////////////////////////////////////////////////////////////////////////////////
//  Implementation of camera3_device_ops
////////////////////////////////////////////////////////////////////////////////
static
int
camera_initialize(
    camera3_device const*           device,
    camera3_callback_ops_t const*   callback_ops
)
{
    status_t status = -ENODEV;
    //
    Cam3Device*const pDev = getDevice(device);
    if  ( pDev )
    {
        status = pDev->i_initialize(callback_ops);
    }
    return  status;
}


static
int
camera_configure_streams(
    camera3_device const*           device,
    camera3_stream_configuration_t* stream_list
)
{
    status_t status = -ENODEV;
    //
    Cam3Device*const pDev = getDevice(device);
    if  ( pDev )
    {
        status = pDev->i_configure_streams(stream_list);
    }
    return  status;
}


static
camera_metadata_t const*
camera_construct_default_request_settings(
    camera3_device const*   device,
    int                     type
)
{
    Cam3Device*const pDev = getDevice(device);
    if  ( pDev )
    {
        return  pDev->i_construct_default_request_settings(type);
    }
    return  NULL;
}


static
int
camera_process_capture_request(
    camera3_device const*       device,
    camera3_capture_request_t*  request
)
{
    status_t status = -ENODEV;
    //
    Cam3Device*const pDev = getDevice(device);
    if  ( pDev )
    {
        status = pDev->i_process_capture_request(request);
    }
    return  status;
}


static
void
camera_dump(
    camera3_device const*   device,
    int                     fd
)
{
    Cam3Device*const pDev = getDevice(device);
    if  ( pDev )
    {
        pDev->i_dump(fd);
    }
}


static
int
camera_flush(
    camera3_device const*   device
)
{
    status_t status = -ENODEV;
    //
    Cam3Device*const pDev = getDevice(device);
    if  ( pDev )
    {
        status = pDev->i_flush();
    }
    return  status;
}


static camera3_device_ops const gCameraDevOps = {
    #define OPS(name) name: camera_##name
    OPS(initialize),
    OPS(configure_streams),
    register_stream_buffers     : NULL,
    OPS(construct_default_request_settings),
    OPS(process_capture_request),
    get_metadata_vendor_tag_ops : NULL,
    OPS(dump),
    OPS(flush),
    #undef  OPS
    reserved: {0},
};


/******************************************************************************
 *
 ******************************************************************************/
Cam3Device::
Cam3Device()
    : ICamDevice()
    , mpModuleCallbacks(NULL)
    , mDevice()
    , mDeviceOps()
{
    MY_LOGD("ctor");
    ::memset(&mDevice, 0, sizeof(mDevice));
    mDevice.priv    = this;
    mDevice.common  = gHwDevice;
    mDevice.ops     = &mDeviceOps;
    mDeviceOps      = gCameraDevOps;
    //
}


/******************************************************************************
 *
 ******************************************************************************/
void
Cam3Device::
onLastStrongRef(const void* /*id*/)
{
    MY_LOGD("");
    i_uninitialize();
}

