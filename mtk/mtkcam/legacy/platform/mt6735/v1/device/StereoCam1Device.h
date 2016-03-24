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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_V1_DEVICE_STEREOCAM1DEVICE_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_V1_DEVICE_STEREOCAM1DEVICE_H_

#include <mtkcam/device/Cam1DeviceBase.h>
//
#include <mtkcam/hal/IResManager.h>
#include <mtkcam/hal/IHalSensor.h>
//
#if '1'==MTKCAM_HAVE_3A_HAL
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/IHal3A.h>
#include <mtkcam/featureio/ISync3A.h>
#endif
//
#include <pthread.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace android {


/******************************************************************************
 *
 ******************************************************************************/
class StereoCam1Device : public Cam1DeviceBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    //
    #define USER_NAME               "StereoCam1Device"
    //
    NSCam::IHalSensor*              mpHalSensor;
    //
#if '1'==MTKCAM_HAVE_3A_HAL
    NS3A::IHal3A*                   mpHal3a_Main;
    NS3A::IHal3A*                   mpHal3a_Main2;
    NS3A::ISync3AMgr*               mpSync3AMgr;
#endif
    bool                            mbThreadRunning;
    bool                            mRet;
    pthread_t                       mThreadHandle;
    //
    int32_t                         mSensorId_Main;
    int32_t                         mSensorId_Main2;
    //
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    virtual                         ~StereoCam1Device();
                                    StereoCam1Device(
                                        String8 const&          rDevName,
                                        int32_t const           i4OpenId
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  [Template method] Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Operations.

    /**
     * [Template method] Called by initialize().
     * Initialize the device resources owned by this object.
     */
    virtual bool                    onInit();

    /**
     * [Template method] Called by uninitialize().
     * Uninitialize the device resources owned by this object. Note that this is
     * *not* done in the destructor.
     */
    virtual bool                    onUninit();

    /**
     * [Template method] Called by startPreview().
     */
    virtual bool                    onStartPreview();

    /**
     * [Template method] Called by stopPreview().
     */
    virtual void                    onStopPreview();

    /**
     * to power on the sensor
     */
    virtual bool                    powerOnSensor();

    /**
     * the init function to be called in the thread
     */
    static void*                    doThreadInit(void* arg);

    /**
     * Wait for initialization in the thread done.
     */
    virtual bool                    waitThreadInitDone();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Cam1Device Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    virtual status_t                setParameters(const char* params);
    virtual int32_t                 queryDisplayBufCount() const    { return 6; }

private:    ////

    int32_t                         getOpenId_Main() const      { return mSensorId_Main; }
    int32_t                         getOpenId_Main2() const     { return mSensorId_Main2; }

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace android
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_V1_DEVICE_STEREOCAM1DEVICE_H_

