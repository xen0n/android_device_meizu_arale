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
#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_UTILS_V3_METADATAPROVIDER_CUSTOM_INFO_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_UTILS_V3_METADATAPROVIDER_CUSTOM_INFO_H_
//
#include <utils/String8.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace android {
namespace NSMetadataProvider {


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
protected:  ////    Data Members.
    int32_t         mSensorIndex;
    int32_t         mSensorDev;
    int32_t         mSensorType;
    char const*     mSensorDrvName;

public:     ////    Instantiation.
    Info(){}
    Info(
        uint_t const    uSensorIndex,
        uint_t const    uSensorDev,
        uint_t const    eSensorType,
        char const*     szSensorDrvName
    )
        : mSensorIndex(uSensorIndex)
        , mSensorDev(uSensorDev)
        , mSensorType(eSensorType)
        , mSensorDrvName(szSensorDrvName)
    {
    }

public:     ////    Accessors.
    int32_t         getDeviceId()      const   { return mSensorIndex; }
    int32_t         getSensorDev()  const   { return mSensorDev; }
    int32_t         getSensorType()    const   { return mSensorType; }
    char const*     getSensorDrvName() const   { return mSensorDrvName; }
};


/******************************************************************************
 *  Static Metadata
 ******************************************************************************/
#define STATIC_METADATA_BEGIN(PREFIX, TYPE, SENSORNAME) \
    extern "C" \
    status_t \
    constructCustStaticMetadata_##PREFIX##_##TYPE##_##SENSORNAME( \
        NSCam::IMetadata&      rMetadata, \
        Info const&     rInfo \
    ) \
    {

#define STATIC_METADATA_END() \
        return  OK; \
    }

#define STATIC_METADATA2_BEGIN(PREFIX, TYPE, SENSORNAME) \
    extern "C" \
    status_t \
    constructCustStaticMetadata_##PREFIX##_##TYPE##_##SENSORNAME( \
        NSCam::IMetadata&      rMetadata\
    ) \
    {

#define STATIC_METADATA_END() \
        return  OK; \
    }


#define PREFIX_FUNCTION_STATIC_METADATA "constructCustStaticMetadata"


/******************************************************************************
 *
 ******************************************************************************/
static
char const*const
kStaticMetadataTypeNames[] =
{
    "LENS",
    "SENSOR",
    "TUNING_3A",
    "FLASHLIGHT",
    "SCALER",
    "FEATURE",
    "CAMERA",
    "REQUEST",
    NULL
};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSMetadataProvider
};  // namespace android
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_UTILS_V3_METADATAPROVIDER_CUSTOM_INFO_H_

