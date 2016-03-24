/*
 * Copyright (C) 2010-2011 The Android Open Source Project
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

#ifndef _MTK_HARDWARE_INCLUDE_MMSDK_IMAGE_TRANSFORM_DEVICE_H_
#define _MTK_HARDWARE_INCLUDE_MMSDK_IMAGE_TRANSFORM_DEVICE_H_

#include "mmsdk_common.h"

/**
 * image transform device HAL, initial version [ IMAGE_TRANSFORM_DEVICE_API_VERSION_1_0 ]
 *
 */
#define IMAGE_TRANSFORM_DEVICE_API_VERSION_1_0 HARDWARE_DEVICE_API_VERSION(1, 0)
/*
 *
 */ 
#define IMAGE_TRANSFORM_DEVICE_API_VERSION_CURRENT IMAGE_TRANSFORM_DEVICE_API_VERSION_1_0

__BEGIN_DECLS


struct imageTransform_device;

/*
 *
 *
 */
typedef struct transformOptions
{
    /*
     * crop rect
     */
    NSCam::MRect rect; 
    /*
     * transform, {flip or rotation} 
     */
    int   transform; 
    /*
     * encode quality {0~100}
     */
    int   encodeQuality; 
    /*
     * is dither enable (true, false}
     */
    bool  isDither; 
    /*
     * sharpness level (0~5)
     */
    int   sharpnessLevel; 

    inline transformOptions()
        :rect(NSCam::MRect(0,0))
        ,transform(0)
        ,encodeQuality(50)
        ,isDither(false)
        ,sharpnessLevel(0)
    {    
    }
    

} transformOptions_t;  

/*
 *
 *
 */
typedef struct imageTransfrom_device_ops {    
    /**
     * applyTransform
     * apply the transform from source iamge to target image by the options 
     */
    bool (*applyTransform)(struct imageTransform_device *, 
                           NSCam::IImageBuffer const *srcImage, 
                           NSCam::IImageBuffer const *destImage, 
                           transformOptions_t const *options); 

} imageTransfrom_device_ops_t;

/*
 *
 *
 */
typedef struct imageTransform_device {
    /**
     * imageTransform_device.common.version must be in the range
     * HARDWARE_DEVICE_API_VERSION(0,0)-(1,FF). IMAGE_TRANSFORM_DEVICE_API_VERSION_1_0 is
     * recommended.
     */
    hw_device_t common;
    imageTransfrom_device_ops_t *ops;
    void *priv;
} imageTransform_device_t;

__END_DECLS

#endif /* #ifdef _MTK_HARDWARE_INCLUDE_MMSDK_IMAGE_TRANSFORM_DEVICE_H_ */
