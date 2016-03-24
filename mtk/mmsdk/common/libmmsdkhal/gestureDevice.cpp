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

#define LOG_TAG "mmsdk/GestureDevice"

#define MTK_LOG_ENABLE 1
#include <cstdlib>
#include <pthread.h>
#include <sync/sync.h>
#include "mmsdkHAL.h"

//

#include <gs_hal_base.h>

#include <common.h>
#include <imageTransform_device.h>


//#define LOG_NDEBUG 0
#include <cutils/log.h>


#include "gestureDevice.h"


#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#warning [FIXME][MMSDK] 
#define GESTURE_WORKING_BUFFER_SIZE    15728640 
#define MAX_NUM_OF_GESTURE_DECTED      15 

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)       ALOGV(fmt"\r\n", ##arg)
#define MY_LOGD(fmt, arg...)       ALOGD(fmt"\r\n", ##arg)
#define MY_LOGI(fmt, arg...)       ALOGI(fmt"\r\n", ##arg)
#define MY_LOGW(fmt, arg...)       ALOGW(fmt"\r\n", ##arg)
#define MY_LOGE(fmt, arg...)       ALOGE(fmt" (%s){#%d:%s}""\r\n", ##arg, __FUNCTION__, __LINE__, __FILE__)


namespace NSMMSdkHAL {

extern "C" {
// Shim passed to the framework to close an opened device.
static int close_device(hw_device_t* dev)
{
    gesture_device_t* gesture_dev = reinterpret_cast<gesture_device_t*>(dev);
    GestureDevice* device = static_cast<GestureDevice*>(gesture_dev->priv);
    return device->close();
}
} // extern "C"

/******************************************************************************
 *
 ******************************************************************************/
GestureDevice::GestureDevice()
    :mpGestureObj(NULL)
    ,mpWorkingBuf(NULL)
    ,mpGestureResult(NULL)
{
    memset(&mDevice, 0, sizeof(mDevice));
    mDevice.common.tag    = HARDWARE_DEVICE_TAG;
    mDevice.common.version = GESTURE_DEVICE_API_VERSION_1_0;
    mDevice.common.close  = close_device;
    mDevice.ops           = const_cast<gesture_device_ops_t*>(&sOps);
    mDevice.priv          = this;
}

/******************************************************************************
 *
 ******************************************************************************/
GestureDevice::~GestureDevice()
{    
}

/******************************************************************************
 *
 ******************************************************************************/
int GestureDevice::open(const hw_module_t *module, hw_device_t **device)
{    
    MY_LOGD("open+"); 
    mDevice.common.module = const_cast<hw_module_t*>(module);
    *device = &mDevice.common;

    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
int GestureDevice::close()
{
    MY_LOGD("close+"); 
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
bool 
GestureDevice::
init(
    eGESTURE_MODE eMode, 
    int imageWidth,     //TODO, remove me 
    int imageHeight,    //TODO, remove me 
    int *maxNumOfDetected
)
{
    MY_LOGD("init mode(%d), (w,h)=(%d,%d)", eMode,imageWidth,imageHeight);  
    bool ret = false; 
    //TODO: currently, only support SW Mode 
    mpGestureObj = halGSBase::createInstance(HAL_GS_OBJ_SW);
    mpWorkingBuf = new unsigned char[GESTURE_WORKING_BUFFER_SIZE];
    if (NULL == mpGestureObj || NULL == mpWorkingBuf)
    {
        MY_LOGE("init fail, null obj"); 
        return false; 
    }
    //
    mpGestureObj->halGSInit(imageWidth,  
                            imageHeight, 
                            mpWorkingBuf, 
                            GESTURE_WORKING_BUFFER_SIZE); 
    //
    mpGestureResult = new MtkCameraFaceMetadata; 
    if (mpGestureResult != NULL)
    {
        MtkCameraFace *gestures = new MtkCameraFace[MAX_NUM_OF_GESTURE_DECTED];
        MtkFaceInfo *gsposInfo = new MtkFaceInfo[MAX_NUM_OF_GESTURE_DECTED];

        if ( NULL != gestures &&  NULL != gsposInfo)
        {
            mpGestureResult->faces = gestures;

            mpGestureResult->posInfo = gsposInfo;
            mpGestureResult->number_of_faces = 0;    
            ret = true;
        }        
    }
    
    *maxNumOfDetected = MAX_NUM_OF_GESTURE_DECTED; 
    return ret; 
} 

/******************************************************************************
 *
 ******************************************************************************/
bool 
GestureDevice::
handDetect(
    NSCam::IImageBuffer const *image,
    unsigned int hand_pose,
    int *numOfDetected,
    hand_detection_result_t *result /*out*/
)
{
    if (NULL == mpGestureObj)
    {
        MY_LOGE("Not init, null gesture obj"); 
        return false; 
    }
    //   
    mpGestureObj->halGSDo((MUINT8*)(image->getBufVA(0)), 0);
    //
    *numOfDetected = mpGestureObj->halGSGetGestureResult(mpGestureResult);
 
    //
    for (int i = 0; i < *numOfDetected; i++)
    {
        result[i].rect.p.x = mpGestureResult->faces[i].rect[0]; 
        result[i].rect.p.y = mpGestureResult->faces[i].rect[1]; 
        result[i].rect.s.w = mpGestureResult->faces[i].rect[2] - mpGestureResult->faces[i].rect[0]; 
        result[i].rect.s.h = mpGestureResult->faces[i].rect[3] - mpGestureResult->faces[i].rect[2]; 
        result[i].pose = hand_pose; 
        result[i].id = mpGestureResult->faces[i].id; 
        result[i].confidence = static_cast <double>(mpGestureResult->faces[i].score); 
        MY_LOGD("result(%d), (x,y,w,h,score)=(%d,%d,%d,%d,%d)",
                result[i].id, 
                result[i].rect.p.x, result[i].rect.p.y, 
                result[i].rect.s.w, result[i].rect.s.h, 
                result[i].confidence); 
    }

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
bool 
GestureDevice::
uninit()
{
    MY_LOGD("unint+"); 
    if (mpGestureObj != NULL)
    {
        mpGestureObj->halGSUninit(); 
        mpGestureObj->destroyInstance();
        mpGestureObj = NULL; 
    }
    //
    if (mpWorkingBuf != NULL)
    {
        delete [] mpWorkingBuf; 
        mpWorkingBuf = NULL; 
    }
    //
    if ( mpGestureResult != NULL )
    {
        if ( mpGestureResult->faces != NULL )
        {
            delete [] mpGestureResult->faces;
            mpGestureResult->faces = NULL;
        }

        if ( mpGestureResult->posInfo != NULL)
        {
            delete [] mpGestureResult->posInfo;
            mpGestureResult->posInfo = NULL;
        }

        delete mpGestureResult;
        mpGestureResult = NULL;
    }    
    MY_LOGD("unint-"); 
    return true; 
}

extern "C" {
// Get handle to camera from device priv data
/******************************************************************************
 *
 ******************************************************************************/
static GestureDevice *gestureDev_to_device(const gesture_device_t *dev)
{
    return reinterpret_cast<GestureDevice*>(dev->priv);
}


/******************************************************************************
 *
 ******************************************************************************/
static bool init(
    struct gesture_device *dev, 
    eGESTURE_MODE eMode, 
    int imageWidth,
    int imageHeight, 
    int *max_num_of_detected    /*out*/
)
{
    return gestureDev_to_device(dev)->init(eMode, imageWidth, imageHeight, max_num_of_detected);
}                  

/******************************************************************************
 *
 ******************************************************************************/
static bool hand_detect (
    struct gesture_device *dev,        
    NSCam::IImageBuffer const *image,
    unsigned int hand_pose,
    int *num_of_detected,      
    hand_detection_result_t *result /*out*/
)
{
    return gestureDev_to_device(dev)->handDetect(image,hand_pose, num_of_detected, result); 
} 

/******************************************************************************
 *
 ******************************************************************************/
static bool uninit (
    struct gesture_device *dev
) 
{
    return gestureDev_to_device(dev)->uninit(); 
}
   
} // extern "C"

const gesture_device_ops GestureDevice::sOps = {
    .init                        = NSMMSdkHAL::init, 
    .hand_detect                 = NSMMSdkHAL::hand_detect, 
    .uninit                      = NSMMSdkHAL::uninit
};

} // namespace NSMMSdkHAL
