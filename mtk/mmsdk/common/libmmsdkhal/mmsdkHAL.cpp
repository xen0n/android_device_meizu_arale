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

#include <mmsdk_common.h>
#include <hardware/hardware.h>




//#define LOG_NDEBUG 0
#define LOG_TAG "MMSDKHAL"
#include <cstdlib>
#include <cutils/log.h>

#include <common.h>

#include <effect_device.h>
#include <gesture_device.h>
#include <imageTransform_device.h>

#include <mmsdk_common.h>

#include "mmsdkHAL.h"

#include "imageTransformDevice.h"
#include "gestureDevice.h"
#include "effectDevice.h"

/******************************************************************************
 *
 ******************************************************************************/
#define ARRAY_OF(a)     (sizeof(a)/sizeof(a[0]))

/*
 * This file serves as the entry point to the HAL.  It contains the module
 * structure and functions used by the framework to load and interface to this
 * HAL, as well as the handles to the individual devices.
 */
using namespace NSCam; 

namespace NSMMSdkHAL {

// 
static MMSdkHAL gMMSDKHAL;

/******************************************************************************
 *
 ******************************************************************************/
static mmsdk_device_info device_info[]
{
    {MMSDK_HARDWARE_GESTURE,             GESTURE_DEVICE_API_VERSION_CURRENT}, 
    {MMSDK_HARDWARE_EFFECT,              EFFECT_DEVICE_API_VERSION_CURRENT},
    {MMSDK_HARDWARE_IMAGE_TRANSFORM,     IMAGE_TRANSFORM_DEVICE_API_VERSION_CURRENT}
}; 

/******************************************************************************
 *
 ******************************************************************************/
MMSdkHAL::MMSdkHAL()    
{
    // Allocate supported device array and instantiate devices
    mpImageTransformDevice = new ImageTransformDevice(); 
    mpGestureDevice = new GestureDevice(); 
    mpEffectDevice = new EffectDevice(); 
}

/******************************************************************************
 *
 ******************************************************************************/
MMSdkHAL::~MMSdkHAL()
{
    delete mpImageTransformDevice; 
    delete mpGestureDevice; 
    delete mpEffectDevice; 
}

/******************************************************************************
 *
 ******************************************************************************/
int MMSdkHAL::get_supported_device_info(int &num_of_devices, struct mmsdk_device_info **info)
{
    //
    num_of_devices = ARRAY_OF(device_info); 
    //
    *info = device_info; 
    
    return 0; 
}

/******************************************************************************
 *
 ******************************************************************************/
int MMSdkHAL::open(const hw_module_t* mod, const char* name, hw_device_t** dev)
{
    ALOGD("module=%p, name=%s, device=%p", mod, name, dev);

    if (*name == '\0') {
        ALOGE("%s: Invalid device name is NULL", __func__);
        return -EINVAL;
    }
    const char *deviceName = name; 

    if(strncmp(deviceName, MMSDK_HARDWARE_GESTURE, strlen(MMSDK_HARDWARE_GESTURE))== 0)
    {
        ALOGD("open gesture device"); 
        return mpGestureDevice->open(mod, dev); 
    }
    else if (strncmp(deviceName, MMSDK_HARDWARE_EFFECT, strlen(MMSDK_HARDWARE_EFFECT))== 0)
    {
        ALOGD("open effect device"); 
        return mpEffectDevice->open(mod, dev); 
    }
    else if (strncmp(deviceName, MMSDK_HARDWARE_IMAGE_TRANSFORM, strlen(MMSDK_HARDWARE_IMAGE_TRANSFORM))== 0)
    {
        ALOGD("open image transform device"); 
        return mpImageTransformDevice->open(mod, dev); 
    }
    else 
    {
        ALOGE("%s, Invalid device name %s", __func__, name);  
        return -EINVAL; 
    }

    return 0; 
}

extern "C" {
/******************************************************************************
 *
 ******************************************************************************/
static int get_supported_device_info(int &num_of_devices, struct mmsdk_device_info **info)
{
    return gMMSDKHAL.get_supported_device_info(num_of_devices, info);
}

/******************************************************************************
 *
 ******************************************************************************/
static int open_dev(const hw_module_t* mod, const char* name, hw_device_t** dev)
{
    return gMMSDKHAL.open(mod, name, dev);
}

/******************************************************************************
 *
 ******************************************************************************/
static hw_module_methods_t gMMSDKModuleMethods = {
    open : open_dev
};

/******************************************************************************
 *
 ******************************************************************************/
mmsdk_module_t HAL_MODULE_INFO_SYM __attribute__ ((visibility("default"))) = {
    common : {
        tag                : HARDWARE_MODULE_TAG,
        module_api_version : MMSDK_MODULE_API_VERSION_1_0,
        hal_api_version    : HARDWARE_HAL_API_VERSION,
        id                 : MMSDK_HARDWARE_MODULE_ID,
        name               : "Default MTK MMSDK HAL",
        author             : "Mediatek inc",
        methods            : &gMMSDKModuleMethods,
        dso                : NULL,
        reserved           : {0},
    },
    get_supported_device_info : get_supported_device_info,
};
} // extern "C"

} // namespace NSMMSdkHAL
