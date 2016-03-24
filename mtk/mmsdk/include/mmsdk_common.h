/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef _MTK_HARDWARE_INCLUDE_MMSDK_COMMON_H_
#define _MTK_HARDWARE_INCLUDE_MMSDK_COMMON_H_

#include <stdint.h>
#include <sys/cdefs.h>
#include <sys/types.h>
#include <hardware/hardware.h>

__BEGIN_DECLS

/**
 * The id of this module
 */
#define MMSDK_HARDWARE_MODULE_ID "mmsdk"

/**
 * Module versioning information for the MMSDK hardware module, based on
 * mmsdk_module_t.common.module_api_version. The two most significant hex
 * digits represent the major version, and the two least significant represent
 * the minor version.
 *
 *******************************************************************************
 * Versions: 1.0  [MMSDK_MODULE_API_VERSION_1_0]
 *
 *
 *******************************************************************************
 */

/**
 * Predefined macros for currently-defined version numbers
 */

/**
 * All module versions <= HARDWARE_MODULE_API_VERSION(1, 0xFF) must be treated
 * as MMSDK_MODULE_API_VERSION_1_0
 */
#define MMSDK_MODULE_API_VERSION_1_0         HARDWARE_MODULE_API_VERSION(1, 0)
#define MMSDK_MODULE_API_VERSION_CURRENT     MMSDK_MODULE_API_VERSION_1_0


/* 
 * MMSDK hardware device name 
 */ 
#define MMSDK_HARDWARE_GESTURE             "gesture"
#define MMSDK_HARDWARE_EFFECT              "effect"
#define MMSDK_HARDWARE_IMAGE_TRANSFORM     "image_transform"


typedef struct mmsdk_device_info {
    /**
     * The device name of xxxx_device_t 
     *
     * Device name 
     */
    char const *device_name; 
    /**
     * The value of xxxx_device_t.common.version.
     *
     * Version information 
     *
     */
    uint32_t device_version;

} mmsdk_device_info_t;


typedef struct mmsdk_module {
    hw_module_t common;

    /**
     * get_supported_device_info:
     *
     * Return the static supported device informations for all supported devices. 
     * This information may not change for a device.
     *
     */
    int (*get_supported_device_info)(int &num_of_devices, struct mmsdk_device_info **info);

    /* reserved for future use */
    void* reserved[8];
} mmsdk_module_t;

__END_DECLS

#endif /* _MTK_HARDWARE_INCLUDE_MMSDK_COMMON_H_ */
