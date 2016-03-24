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

#ifndef _MTK_HARDWARE_INCLUDE_MMSDK_DEVICE_GESTURE_H_
#define _MTK_HARDWARE_INCLUDE_MMSDK_DEVICE_GESTURE_H_

#include "mmsdk_common.h"

/**
 * Gesture device HAL, initial version [ GESTURE_DEVICE_API_VERSION_1_0 ]
 *
 */
#define GESTURE_DEVICE_API_VERSION_1_0 HARDWARE_DEVICE_API_VERSION(1, 0)

// 
#define GESTURE_DEVICE_API_VERSION_CURRENT GESTURE_DEVICE_API_VERSION_1_0


__BEGIN_DECLS


struct gesture_device;

/*
 * HAND pose 
 *
 */
enum eHAHND_POSE
{
    HAND_POSE_OPEN     = 0x00000000, 
    HAND_POSE_VICTORY  = 0x00000001, 
}; 

/*
 *  HAND detection mode 
 *
 */ 
enum eGESTURE_MODE
{
    GESTURE_SW_ONLY_MODE, 
    GESTURE_SW_HW_HYBRID_MODE, 
    GESTURE_HW_ONLY_MODE, 
}; 


/*
 * hand detection listener 
 *
 */
typedef struct hand_detection_result
{
    /*
     * hand_detection_result rect
     */
    NSCam::MRect rect; 
    /*
     * confidence value 
     */
    float   confidence; 
    /*
     * Identifier associated with this dtection.
     */
    int   id; 
    /*
     * detected pose 
     */
    int  pose; 
}hand_detection_result_t;

/*
 * gesture device operation 
 *
 */
typedef struct gesture_device_ops 
{     
    /*
     * init gesture mode 
     */  
    bool (*init)(struct gesture_device *, 
                 eGESTURE_MODE eMode, 
                 int imageWidth,
                 int imageHeight, 
                 int *max_num_of_detected    /*out*/);                  
    /*
     *
     */
    bool (*hand_detect)(struct gesture_device *,        
                        NSCam::IImageBuffer const *image,
                        unsigned int hand_pose,
                        int *num_of_detected,           /*out*/ 
                        hand_detection_result_t *result /*out*/
                   ); 
    /*
     *
     */
    bool (*uninit)(struct gesture_device *); 
   

} gesture_device_ops_t;

/*
 * gesture device 
 */
typedef struct gesture_device {
    /**
     * gesture_device.common.version must be in the range
     * HARDWARE_DEVICE_API_VERSION(0,0)-(1,FF). GESTURE_DEVICE_API_VERSION_1_0 is
     * recommended.
     */
    hw_device_t common;
    gesture_device_ops_t *ops;
    void *priv;
} gesture_device_t;

__END_DECLS

#endif /* #ifdef _MTK_HARDWARE_INCLUDE_MMSDK_DEVICE_GESTURE_H_ */
