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

#ifndef _MTK_HARDWARE_INCLUDE_MMSDK_DEVICE_EFFECT_H_
#define _MTK_HARDWARE_INCLUDE_MMSDK_DEVICE_EFFECT_H_

#include "mmsdk_common.h"

/**
 * Effect device HAL, initial version [ EFFECT_DEVICE_API_VERSION_1_0 ]
 *
 */
#define EFFECT_DEVICE_API_VERSION_1_0 HARDWARE_DEVICE_API_VERSION(1, 0)
// 
#define EFFECT_DEVICE_API_VERSION_CURRENT EFFECT_DEVICE_API_VERSION_1_0

__BEGIN_DECLS


struct effect_device;

/*
 *
 */
enum eEFFECT_MODE
{
    EFFECT_MODE_VIDEO, 
    EFFECT_MODE_STILL
}; 


/*
 * face beauty parameter 
 */ 
#define FACE_EFFECT_SMOOTH_LEVEL            "SMOOTH_LEVEL" 
#define FACE_EFFECT_SKIN_COLOR              "SKIN_COLOR_LEVEL"
#define FACE_EFFECT_ENLARGE_EYE_LEVEL       "ENLARGE_EYE_LEVEL"
#define FACE_EFFECT_SLIM_FACE_LEVEL         "SLIM_FACE_LEVEL" 
#define FACE_EFFECT_MODE                    "EFFECT_MODE"

/*
 *
 */
typedef struct effect_device_ops {    
    /**
     * set_parameters
     *
     */
    bool (*create_effect)(struct effect_device *,  
                          char const* effect_name, 
                          eEFFECT_MODE eMode,   
                          void *cookie  
                          ); 

    /*
     * get the effect name 
     */
    char *(*get_effect_name)(struct effect_device *, void *cookie); 

    /*
     * set the parameter 
     *
     */
    bool (*set_parameters)(struct effect_device *, 
                           const char *param, 
                           const void *value,
                           void *cookie
                          );     

    /**
     * applyTransform
     * apply the transform from source iamge to target image by the options 
     */
    bool (*apply)(struct effect_device *, 
                  NSCam::IImageBuffer const *srcImage, 
                  NSCam::IImageBuffer const *destImage,  
                  void *cookie 
                  ); 
    

    /**
     * release the hardware resource owned by this object. 
     *
     */
    bool (*release)(struct effect_device*, void *cookie); 

} effect_device_ops_t;

typedef struct effect_device {
    /**
     * effect_device.common.version must be in the range
     * HARDWARE_DEVICE_API_VERSION(0,0)-(1,FF). EFFECT_DEVICE_API_VERSION_1_0 is
     * recommended.
     */
    hw_device_t common;
    effect_device_ops_t *ops;
    void *priv;
} effect_device_t;

__END_DECLS

#endif /* #ifdef _MTK_HARDWARE_INCLUDE_MMSDK_DEVICE_EFFECT_H_ */
