/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_CAM3DEVICE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_CAM3DEVICE_H_
//
#include <utils/String8.h>
//
#include <hardware/camera3.h>
#include "ICamDevice.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {


/******************************************************************************
 *
 ******************************************************************************/
class Cam3Device : public ICamDevice
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.
    typedef android::status_t       status_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.

    camera_module_callbacks_t const*mpModuleCallbacks;
    camera3_device                  mDevice;
    camera3_device_ops              mDeviceOps;         //  which is pointed to by mDevice.ops.

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    //
    static  inline Cam3Device*      getDevice(camera3_device const* device)
                                    {
                                        return (NULL == device)
                                                ? NULL
                                                : (Cam3Device*)((device)->priv)
                                                ;
                                    }

    static  inline Cam3Device*      getDevice(hw_device_t* device)
                                    {
                                        return  getDevice((camera3_device const*)device);
                                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Instantiation.
    virtual                         ~Cam3Device() {}
                                    Cam3Device();

    virtual void                    onLastStrongRef(const void* id);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Initialization.

    virtual status_t                i_closeDevice()                         = 0;

    /**
     * One-time initialization to pass framework callback function pointers to
     * the HAL. Will be called once after a successful open() call, before any
     * other functions are called on the camera3_device_ops structure.
     *
     * Return values:
     *
     *  0:     On successful initialization
     *
     * -ENODEV: If initialization fails. Only close() can be called successfully
     *          by the framework after this.
     */
    virtual status_t                i_initialize(
                                        camera3_callback_ops const* callback_ops
                                    )                                       = 0;

    /**
     * Uninitialize the device resources owned by this object. Note that this
     * is *not* done in the destructor.
     *
     * This may be called at any time, although the call may block until all
     * in-flight captures have completed (all results returned, all buffers
     * filled). After the call returns, no more callbacks are allowed.
     */
    virtual status_t                i_uninitialize()                        = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Stream management

    virtual status_t                i_configure_streams(
                                        camera3_stream_configuration_t* stream_list
                                    )                                       = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Request creation and submission

    virtual camera_metadata const*  i_construct_default_request_settings(
                                        int type
                                    )                                       = 0;

    virtual status_t                i_process_capture_request(
                                        camera3_capture_request_t* request
                                    )                                       = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Miscellaneous methods

    virtual status_t                i_flush()                               = 0;

    virtual void                    i_dump(int fd)                          = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ICamDevice Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual hw_device_t const*      get_hw_device() const { return &mDevice.common; }

    virtual void                    set_hw_module(hw_module_t const* module)
                                    {
                                        mDevice.common.module = const_cast<hw_module_t*>(module);
                                    }

    virtual void                    set_module_callbacks(camera_module_callbacks_t const* callbacks)
                                    {
                                        mpModuleCallbacks = callbacks;
                                    }
};
};  //namespace NSCam


/******************************************************************************
 *
 ******************************************************************************/
extern "C"
NSCam::Cam3Device*
createCam3Device(
    android::String8 const  s8ClientAppMode,
    int32_t const           i4OpenId
);


/******************************************************************************
 *
 ******************************************************************************/
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_DEVICE_CAM3DEVICE_H_

