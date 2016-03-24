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

#define LOG_TAG "MtkCam/TemplateRequest"
//
#include "MyUtils.h"
#include <hardware/camera3.h>
//
#include <dlfcn.h>

// converter
#include <mtkcam/metadata/client/TagMap.h>
#include <mtkcam/metadata/IMetadataTagSet.h>
#include <mtkcam/metadata/IMetadataConverter.h>
#include <mtkcam/metadata/IMetadataProvider.h>
#include <system/camera_metadata.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/metadata/mtk_metadata_types.h>
#include <mtkcam/hal/IHalSensor.h>

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
status_t
TemplateRequest::
impConstructRequestMetadata_by_SymbolName(
    String8 const&      s8Symbol,
    IMetadata&          metadata,
    int const           requestType
)
{
typedef status_t (*PFN_T)(
        IMetadata &         metadata,
        int const           requestType
    );
    //
    PFN_T pfn = (PFN_T)::dlsym(RTLD_DEFAULT, s8Symbol.string());
    if  ( ! pfn ) {
        MY_LOGW("%s not found", s8Symbol.string());
        return  NAME_NOT_FOUND;
    }
    //
    status_t const status = pfn(metadata, requestType);
    MY_LOGW_IF(OK != status, "%s returns status[%s(%d)]", s8Symbol.string(), ::strerror(-status), -status);
    //
    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
TemplateRequest::
impConstructRequestMetadata(
    IMetadata&      metadata,
    int const       requestType
)
{
    status_t status = OK;
    //
#if 1
    String8 const s8Symbol_Sensor = String8::format("%s_%s", PREFIX_FUNCTION_REQUEST_METADATA, mInfo.getSensorDrvName());
    status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Sensor, metadata, requestType);
    if  ( OK == status ) {
        return  OK;
    }
#endif
    //
    String8 const s8Symbol_Common = String8::format("%s_%s", PREFIX_FUNCTION_REQUEST_METADATA, "COMMON");
    status = impConstructRequestMetadata_by_SymbolName(s8Symbol_Common, metadata, requestType);
    if  ( OK == status ) {
        return  OK;
    }
    //
    MY_LOGE("Fail to get %s", s8Symbol_Common.string());
    //
    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
static bool setTagInfo(IMetadataTagSet &rtagInfo)
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


/******************************************************************************
 *
 ******************************************************************************/
status_t
TemplateRequest::
constructRequestMetadata(
    int const           requestType,
    camera_metadata*&   rpMetadata,
    IMetadata&          rMtkMetadata
)
{
    MY_LOGD("constructRequestMetadata");

    status_t status = OK;

    //-----(1)-----//
    //get static informtation from customization (with camera_metadata format)
    //calculate its entry count and data count
    if  ( OK != (status = impConstructRequestMetadata(rMtkMetadata, requestType)) ) {
        MY_LOGE("Unable evaluate the size for camera static info - status[%s(%d)]\n", ::strerror(-status), -status);
        return  status;
    }
    MY_LOGD("Allocating %d entries from customization", rMtkMetadata.count());

    //calculate its entry count and data count
    // init converter
    IMetadataTagSet tagInfo;
    setTagInfo(tagInfo);
    sp<IMetadataConverter> pConverter = IMetadataConverter::createInstance(tagInfo);
    size_t entryCount = 0;
    size_t dataCount = 0;
    MBOOL ret = pConverter->get_data_count(rMtkMetadata, entryCount, dataCount);
    if ( ret != OK )
    {
        MY_LOGE("get Imetadata count error\n");
        return UNKNOWN_ERROR;
    }
    MY_LOGD("Allocating %d entries, %d extra bytes from HAL modules", entryCount, dataCount);

    //-----(2)-----//
    // overwrite
    updateData(rMtkMetadata);

    //-----(3)-----//
    // convert to android metadata
    pConverter->convert(rMtkMetadata, rpMetadata);
    ::sort_camera_metadata(rpMetadata);

    return  status;
}


/******************************************************************************
 *
 ******************************************************************************/
void
TemplateRequest::
updateData(IMetadata &rMetadata)
{
    sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(mInfo.getDeviceId());
    IMetadata static_meta = pMetadataProvider->geMtktStaticCharacteristics();
    {
        MRect cropRegion;
        IMetadata::IEntry active_array_entry = static_meta.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
        if( !active_array_entry.isEmpty() ) {
            cropRegion = active_array_entry.itemAt(0, Type2Type<MRect>());
            cropRegion.p.x=0;
            cropRegion.p.y=0;
            IMetadata::IEntry entry(MTK_SCALER_CROP_REGION);
            entry.push_back(cropRegion, Type2Type< MRect >());
            rMetadata.update(MTK_SCALER_CROP_REGION, entry);
         }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
TemplateRequest::
onCreate(int iOpenId)
{
    MY_LOGD("+");
    //
    int32_t sensorType = IHalSensorList::get()->queryType(iOpenId);
    const char* sensorDrvName = IHalSensorList::get()->queryDriverName(iOpenId);
    mInfo = Info(iOpenId, sensorType, sensorDrvName);


    //  Standard template types
    for (int type = CAMERA3_TEMPLATE_PREVIEW; type < CAMERA3_TEMPLATE_COUNT; type++)
    {
        camera_metadata* metadata = NULL;
        IMetadata mtkMetadata;
        status_t status = constructRequestMetadata(type, metadata, mtkMetadata);
        if  ( OK != status || NULL == metadata || mtkMetadata.isEmpty()) {
            MY_LOGE("constructRequestMetadata - type:%#x metadata:%p status[%s(%d)]", type, metadata, ::strerror(-status), -status);
            return status;
        }
        //
        mMapRequestTemplate.add(type, metadata);

        mMapRequestTemplateMetadata.add(type, mtkMetadata);
    }

#if 0
    //  vendor-defined request templates
    for (int type = CAMERA3_VENDOR_TEMPLATE_START; type < CAMERA3_VENDOR_TEMPLATE_COUNT; type++)
    {
        camera_metadata* metadata = NULL;
        status = constructRequestMetadata(type, metadata);
        if  ( OK != status || NULL == metadata ) {
            MY_LOGE("constructRequestMetadata - type:%#x metadata:%p status[%s(%d)]", type, metadata, ::strerror(-status), -status);
            return  status;
        }
        //
        MapRequestTemplate.add(type, metadata);
    }
#endif
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
camera_metadata const*
TemplateRequest::
getData(int requestType)
{
    return mMapRequestTemplate.valueFor(requestType);
}


/******************************************************************************
 *
 ******************************************************************************/
IMetadata const&
TemplateRequest::
getMtkData(int requestType)
{
    return mMapRequestTemplateMetadata.valueFor(requestType);
}


/******************************************************************************
 *
 ******************************************************************************/
ITemplateRequest*
ITemplateRequest::
getInstance(int iOpenId)
{
    TemplateRequest* p = new TemplateRequest();
    if(p != NULL) {
        p->onCreate(iOpenId);
    }
    return p;
}

