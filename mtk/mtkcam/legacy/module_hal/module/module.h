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

#ifndef _MTK_HARDWARE_MTKCAM_MODULE_MODULE_H_
#define _MTK_HARDWARE_MTKCAM_MODULE_MODULE_H_

/******************************************************************************
 *
 ******************************************************************************/
#include <device/ICamDeviceManager.h>


////////////////////////////////////////////////////////////////////////////////
//  Implementation of hw_module_methods_t
////////////////////////////////////////////////////////////////////////////////
static
int
open_device(hw_module_t const* module, const char* name, hw_device_t** device)
{
    return  NSCam::getCamDeviceManager()->open(device, module, name);
}


static
hw_module_methods_t*
get_module_methods()
{
    static
    hw_module_methods_t
    _methods =
    {
        open: open_device
    };

    return  &_methods;
}


////////////////////////////////////////////////////////////////////////////////
//  Implementation of camera_module_t
////////////////////////////////////////////////////////////////////////////////
static
int
get_number_of_cameras(void)
{
    return  NSCam::getCamDeviceManager()->getNumberOfDevices();
}


static
int
get_camera_info(int cameraId, camera_info* info)
{
    return  NSCam::getCamDeviceManager()->getDeviceInfo(cameraId, *info);
}


static
int
set_callbacks(camera_module_callbacks_t const* callbacks)
{
    return  NSCam::getCamDeviceManager()->setCallbacks(callbacks);
}


static
void
get_vendor_tag_ops(vendor_tag_ops_t* ops)
{
    NSCam::getCamDeviceManager()->getVendorTagOps(ops);
}


static
int
open_legacy(
    const struct hw_module_t* module,
    const char* id,
    uint32_t halVersion,
    struct hw_device_t** device
)
{
    return  NSCam::getCamDeviceManager()->open(device, module, id, halVersion);
}


static
int
set_torch_mode(
    const char* camera_id,
    bool enabled
)
{
    return NSCam::getCamDeviceManager()->setTorchMode(atoi(camera_id), enabled);
}


static
camera_module
get_camera_module()
{
    camera_module module = {
        common:{
             tag                    : HARDWARE_MODULE_TAG,
             #if (PLATFORM_SDK_VERSION >= 21)
             module_api_version     : CAMERA_MODULE_API_VERSION_2_4,
             #else
             module_api_version     : CAMERA_DEVICE_API_VERSION_1_0,
             #endif
             hal_api_version        : HARDWARE_HAL_API_VERSION,
             id                     : CAMERA_HARDWARE_MODULE_ID,
             name                   : "MediaTek Camera Module",
             author                 : "MediaTek",
             methods                : get_module_methods(),
             dso                    : NULL,
             reserved               : {0},
        },
        get_number_of_cameras       : get_number_of_cameras,
        get_camera_info             : get_camera_info,
        set_callbacks               : set_callbacks,
        get_vendor_tag_ops          : get_vendor_tag_ops,
        #if (PLATFORM_SDK_VERSION >= 21)
        open_legacy                 : open_legacy,
        #endif
        set_torch_mode              : set_torch_mode,
        init                        : NULL,
        reserved                    : {0},
    };
    return  module;
};


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HARDWARE_MTKCAM_MODULE_MODULE_H_

