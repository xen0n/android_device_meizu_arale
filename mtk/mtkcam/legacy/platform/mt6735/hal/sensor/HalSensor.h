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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_HALSENSOR_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_HALSENSOR_H_
//
#include <mtkcam/hal/IHalSensor.h>
#include "seninf_drv.h"
#include "sensor_drv.h"
#include <mtkcam/exif/IBaseCamExif.h>



/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSHalSensor {

class HalSensorList;



/******************************************************************************
 *  Hal Sensor.
 ******************************************************************************/
class HalSensor : public IHalSensor
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IHalSensor Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    /**
     * Destroy this instance created from IHalSensorList::createSensor.
     */
    virtual MVOID                   destroyInstance(
                                        char const* szCallerName
                                    );

public:     ////                    Operations.

    /**
     * Turn on/off the power of sensor(s).
     */
        virtual MBOOL                    powerOn(
                                               char const* szCallerName,
                                               MUINT const uCountOfIndex,
                                               MUINT const*pArrayOfIndex
                                        );
        virtual MBOOL                    powerOff(
                                              char const* szCallerName,
                                              MUINT const uCountOfIndex,
                                              MUINT const*pArrayOfIndex
                                        );


    /**
     * Configure the sensor(s).
     */
    virtual MBOOL                   configure(
                                        MUINT const         uCountOfParam,
                                        ConfigParam const*  pArrayOfParam
                                    );
    /**
     * Configure the sensor(s).
     */
    virtual MINT                     sendCommand(
                                        MUINT sensorDevIdx,
                                        MUINTPTR cmd,
                                        MUINTPTR arg1,
                                        MUINTPTR arg2,
                                        MUINTPTR arg3
                                    );

    /**
     * Query sensorDynamic information after calling configure
      */
    virtual MBOOL                 querySensorDynamicInfo(
                                      MUINT32 sensorIdx,
                                      SensorDynamicInfo *pSensorDynamicInfo
                                   );
   /**
     * Sensor set exif debug information.
     */

    virtual MINT32                  setDebugInfo(IBaseCamExif *pIBaseCamExif);

    /**
    * release I2C HW sync trigger lock.
     */
    virtual MINT32                releaseI2CTrigLock();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  HalSensor Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    virtual                         ~HalSensor();
                                    HalSensor();

public:     ////                    Operations.
    virtual MBOOL                   isMatch(
                                        SortedVector<MUINT>const& vSensorIndex
                                    ) const;

    virtual MVOID                   onDestroy();
    virtual MBOOL                   onCreate(
                                        SortedVector<MUINT>const& vSensorIndex
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
                                    class PerData : public RefBase
                                    {
                                    private:    ////        Disallowed.
                                                            PerData(PerData const&);
                                        PerData&            operator=(PerData const&);

                                    public:     ////        Operations.
                                                            PerData();

                                    public:     ////        Fields.
                                    };

    typedef sp<PerData>                     PerDataPtr;
    typedef KeyedVector<MUINT, PerDataPtr>  DataMap_t;
    static  PerDataPtr                      kpDefaultPerData;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


protected:  ////                    Data Members.
    mutable Mutex                   mMutex;
    DataMap_t                       mSensorDataMap;
    //volatile MINT32                 mPowerRefCount;
    //int                                   meSensorDev;


protected:  ////                    Operations.
    virtual PerDataPtr              queryPerData(MUINT iSensorIndex, char const* szCaller = "") const;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Control Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

protected:
    SensorDrv*                  mpSensorDrv;
    SeninfDrv*                  mpSeninfDrv;

protected:
    virtual MINT                     setTgPhase(MINT32 sensorIdx, MINT32 pcEn);
    virtual MINT                     setSensorIODrivingCurrent(MINT32 sensorIdx);
    virtual MVOID                   pixelModeArrange(MUINT currSensorDev);
    virtual MINT                    GetpixelMode(MUINT currSensorDev, MUINT scenario, MUINT fps);
    virtual MINT                    seninfControl(MUINT8 enable, MUINT32 currSensorDev);


};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSHalSensor
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_HALSENSOR_H_

