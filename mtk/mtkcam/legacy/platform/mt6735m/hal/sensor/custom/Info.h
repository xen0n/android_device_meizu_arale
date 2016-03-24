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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_CUSTOM_INFO_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_CUSTOM_INFO_H_
//
#include <utils/String8.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSHalSensor {


/******************************************************************************
 *
 ******************************************************************************/
void
showCustInfo();


/******************************************************************************
 *  Info
 ******************************************************************************/
class Info
{
public:     ////                    Accessors.
    uint_t                          getDeviceId()   const   { return muSensorIndex; }
    uint_t                          getSensorType()    const   { return meSensorType; }
    uint_t                          getFacing() const { return muFacing; }
    android::String8 const&         getSensorDrvName() const   { return ms8SensorDrvName; }

public:     ////                    Instantiation.
                                    Info(
                                        uint_t const    uSensorIndex,
                                        uint_t const    eSensorType,
                                        char const*     szSensorDrvName,
                                        uint_t const    uFacing
                                    )
                                        : muSensorIndex(uSensorIndex)
                                        , meSensorType(eSensorType)
                                        , ms8SensorDrvName(android::String8(szSensorDrvName))
                                        , muFacing(uFacing)
                                    {
                                    }

protected:  ////                    Data Members.
    uint_t                          muSensorIndex;
    uint_t                          meSensorType;       //NSSensorType::Type
    uint_t                          muFacing;
    android::String8                ms8SensorDrvName;   //SENSOR_DRVNAME_xxx

};


/******************************************************************************
 *  Static Metadata
 ******************************************************************************/
#define STATIC_METADATA_BEGIN(PREFIX, TYPE, SENSORNAME) \
    extern "C" \
    MBOOL \
    constructCustStaticMetadata_##PREFIX##_##TYPE##_##SENSORNAME( \
        NSCam::IMetadata&      rMetadata, \
        Info const&     rInfo \
    ) \
    {

#define STATIC_METADATA_END() \
        return MTRUE; \
    }

#define STATIC_METADATA2_BEGIN(PREFIX, TYPE, SENSORNAME) \
    extern "C" \
    MBOOL \
    constructCustStaticMetadata_##PREFIX##_##TYPE##_##SENSORNAME( \
        NSCam::IMetadata&      rMetadata, \
        Info const&     rInfo \
    ) \
    {

#define STATIC_METADATA_END() \
        return MTRUE; \
    }


#define PREFIX_FUNCTION_STATIC_METADATA "constructCustStaticMetadata"

/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSHalSensor
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_HAL_SENSOR_CUSTOM_INFO_H_

