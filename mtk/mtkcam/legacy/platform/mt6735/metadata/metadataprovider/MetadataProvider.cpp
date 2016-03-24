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

#define LOG_TAG "MtkCam/MetadataProvider"
//
#include "MyUtils.h"
#include <hardware/camera3.h>
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/metadata/IMetadataTagSet.h>
#include <mtkcam/metadata/IMetadataConverter.h>
#include <mtkcam/metadata/client/TagMap.h>
#include <system/camera_metadata.h>
#include <mtkcam/metadata/mtk_metadata_types.h>


//
/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
static bool setTagInfo(IMetadataTagSet &rtagInfo);


/******************************************************************************
 *
 ******************************************************************************/
IMetadataProvider*
IMetadataProvider::
create(
    int32_t const   i4OpenId
)
{
    MetadataProvider* p = new MetadataProvider(i4OpenId);
    if  ( ! p )
    {
        MY_LOGE("No Memory");
        return  NULL;
    }
    //
    if  ( OK != p->onCreate() )
    {
        MY_LOGE("onCreate");
        delete  p;
        return  NULL;
    }
    //
    return  p;
}


/******************************************************************************
 *
 ******************************************************************************/
MetadataProvider::
~MetadataProvider()
{
    MY_LOGD("+ OpenId:%d", mInfo.getDeviceId());
    //
    if  ( mpStaticCharacteristics )
    {
        ::free_camera_metadata(mpStaticCharacteristics);
        mpStaticCharacteristics = NULL;
    }
    //
    MY_LOGD("- OpenId:%d", mInfo.getDeviceId());
}


/******************************************************************************
 *
 ******************************************************************************/
MetadataProvider::
MetadataProvider(
    int32_t const   i4OpenId
)
    : mpStaticCharacteristics(NULL)
    , mpHALMetadata()
{
    int32_t sensorType = IHalSensorList::get()->queryType(i4OpenId);
    int32_t sensorDev = IHalSensorList::get()->querySensorDevIdx(i4OpenId);
    const char* sensorDrvName = IHalSensorList::get()->queryDriverName(i4OpenId);
    //
    mInfo = Info(i4OpenId, sensorDev, sensorType, sensorDrvName);
    //
    showCustInfo();
}

/******************************************************************************
 *
 ******************************************************************************/
uint32_t
MetadataProvider::
getDeviceVersion() const
{
    return mpHALMetadata.entryFor(MTK_HAL_VERSION).itemAt(0, Type2Type< MINT32 >());
}


/******************************************************************************
 *
 ******************************************************************************/
int
MetadataProvider::
getDeviceFacing() const
{
    return mpHALMetadata.entryFor(MTK_SENSOR_INFO_FACING).itemAt(0, Type2Type< MUINT8 >());
}


/******************************************************************************
 *
 ******************************************************************************/
int
MetadataProvider::
getDeviceWantedOrientation() const
{
    if (mpHALMetadata.entryFor(MTK_SENSOR_INFO_WANTED_ORIENTATION).isEmpty())
        return mInfo.getDeviceId() == 0 ? 90 : 270;

    return mpHALMetadata.entryFor(MTK_SENSOR_INFO_WANTED_ORIENTATION).itemAt(0, Type2Type< MINT32 >());
}


/******************************************************************************
 *
 ******************************************************************************/
int
MetadataProvider::
getDeviceSetupOrientation() const
{
    return mpHALMetadata.entryFor(MTK_SENSOR_INFO_ORIENTATION).itemAt(0, Type2Type< MINT32 >());
}


/******************************************************************************
 *
 ******************************************************************************/
int
MetadataProvider::
getDeviceHasFlashLight() const
{
    if (mpHALMetadata.entryFor(MTK_FLASH_INFO_AVAILABLE).isEmpty())
        return 0;

    return mpHALMetadata.entryFor(MTK_FLASH_INFO_AVAILABLE).itemAt(0, Type2Type< MUINT8 >());
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataProvider::
onCreate()
{
    status_t status = OK;

    //prepare data for TagInfo
    IMetadataTagSet mtagInfo;
    setTagInfo(mtagInfo);

    //create IMetadataConverter
    sp<IMetadataConverter> pMetadataConverter = IMetadataConverter::createInstance(mtagInfo);

    status = constructStaticMetadata(pMetadataConverter, mpStaticCharacteristics, mpHALMetadata);
    if  ( OK != status ) {
        MY_LOGE("constructStaticMetadata - status[%s(%d)]", ::strerror(-status), -status);
        return  status;
    }

    return  status;
}

static
bool setTagInfo(IMetadataTagSet &rtagInfo)
{
    #define _IMP_SECTION_INFO_(...)
    #undef  _IMP_TAG_INFO_
    #define _IMP_TAG_INFO_(_tag_, _type_, _name_) \
        rtagInfo.addTag(_tag_, _name_, Type2TypeEnum<_type_>::typeEnum);
    #include <mtkcam/metadata/client/mtk_metadata_tag_info.inl>
    #undef  _IMP_TAG_INFO_


    #undef _IMP_TAGCONVERT_
    #define _IMP_TAGCONVERT_(_android_tag_, _mtk_tag_) \
        rtagInfo.addTagMap(_android_tag_, _mtk_tag_);
    #if (PLATFORM_SDK_VERSION >= 21)
    ADD_ALL_MEMBERS;
    #endif

    #undef _IMP_TAGCONVERT_

    return MTRUE;
}


