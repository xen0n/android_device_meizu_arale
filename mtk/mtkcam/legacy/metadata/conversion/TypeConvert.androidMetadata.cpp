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

#define LOG_TAG "MtkCam/MetadataConvert"
//
#define MTK_LOG_ENABLE 1
#include "MyUtils.h"
#include <system/camera_metadata.h>
#include <MetadataConverter.h>
#include <metadata/IMetadataTagSet.h>
#include <metadata/mtk_metadata_types.h>

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

//#define MY_LOGD(fmt, arg...)    do {ALOGD(LOG_TAG fmt, ##arg);printf(LOG_TAG fmt "\n", ##arg);} while (0)
//#define MY_LOGE(fmt, arg...)    do {ALOGE("error" LOG_TAG fmt, ##arg);printf(LOG_TAG fmt "\n", ##arg);} while (0)

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
template<class T>
static void
Entry2Array(Vector<T> &storage, IMetadata::IEntry const& entry)
{
    for(uint32_t i = 0; i < entry.count(); i++)
    {
        storage.push_back(entry.itemAt(i, Type2Type< T >() ));
    }
}


template<class T>
static void
MRect2Array(Vector<T> &storage, IMetadata::IEntry const& entry)
{
    for(uint32_t i = 0; i < entry.count(); i++)
    {
        MRect src_rect = entry.itemAt(i, Type2Type< MRect >());
        storage.push_back(src_rect.p.x);
        storage.push_back(src_rect.p.y);
        storage.push_back(src_rect.s.w);
        storage.push_back(src_rect.s.h);
    }
}


template<class T>
static void
MSize2Array(Vector<T> &storage, IMetadata::IEntry const& entry)
{
    for(uint32_t i = 0; i < entry.count(); i++)
    {
        MSize src_size = entry.itemAt(i, Type2Type< MSize >());
        storage.push_back(src_size.w);
        storage.push_back(src_size.h);
    }
}


#define ASSIGN_ENTRY_TO_VOID_PTR(_type_, _mtk_tag_, _android_tag_, _dst_buffer_, _src_entry_, _data_count_) \
if (getTagInfo()->getType(_mtk_tag_) == TYPE_##_type_) { \
    Vector<_type_> storage; \
    Entry2Array(storage, _src_entry_); \
    if(update(_dst_buffer_, _android_tag_, &storage[0], _data_count_)!=OK){ \
        return MFALSE; \
    } \
}


#define NORMAL_COPY_TO_ANDROID(_mtk_tag_, _android_tag_, _dst_buffer_, _src_entry_, _data_count_) \
    ASSIGN_ENTRY_TO_VOID_PTR(MUINT8, _mtk_tag_, _android_tag_, _dst_buffer_, _src_entry_, _data_count_); \
    ASSIGN_ENTRY_TO_VOID_PTR(MINT32, _mtk_tag_, _android_tag_, _dst_buffer_, _src_entry_, _data_count_); \
    ASSIGN_ENTRY_TO_VOID_PTR(MFLOAT, _mtk_tag_, _android_tag_, _dst_buffer_, _src_entry_, _data_count_); \
    ASSIGN_ENTRY_TO_VOID_PTR(MINT64, _mtk_tag_, _android_tag_, _dst_buffer_, _src_entry_, _data_count_); \
    ASSIGN_ENTRY_TO_VOID_PTR(MDOUBLE, _mtk_tag_, _android_tag_, _dst_buffer_, _src_entry_, _data_count_); \
    ASSIGN_ENTRY_TO_VOID_PTR(MRational, _mtk_tag_, _android_tag_, _dst_buffer_, _src_entry_, _data_count_);


// IMetadata --> camera_metadata
MBOOL
MetadataConverter::
convert(const IMetadata &rMetadata, camera_metadata* &pDstMetadata) const
{
    CAM_LOGV("Convert from IMetadata to camera_metadata");

    //allocate
    size_t entryCount = 0;
    size_t dataCount = 0;
    MBOOL ret = get_data_count(rMetadata, entryCount, dataCount);
    if ( ret != OK )
    {
        MY_LOGE("get Imetadata count error\n");
        return MFALSE;
    }
    MY_LOGD("Allocating %d entries, %d extra bytes from HAL modules", entryCount, dataCount);

    pDstMetadata = ::allocate_camera_metadata(entryCount, dataCount);
    if ( !pDstMetadata )
    {
        MY_LOGE(
            "Unable to allocate camera static info (%d entries, %d bytes extra data)\n",
            entryCount, dataCount
        );
        return  MFALSE;
     }

    //start to convert
    for (uint32_t i = 0; i < rMetadata.count(); i++)
    {
        //(1) get tag by index, and skip it if it doesn't exist in android-metadata-tag list.
        MUINT32 mtk_tag = rMetadata.entryAt(i).tag();

        //(2) map to tag defined by android; return if undefined.
        #if (PLATFORM_SDK_VERSION >= 21)
        MUINT32 android_tag = getTagInfo()->getAndroidTag((MUINT32)mtk_tag);
        #else
        MUINT32 android_tag = 0;
        #endif
        if (android_tag == IMetadata::IEntry::BAD_TAG){
            CAM_LOGD("%s: Tag 0x%x not found in Android Metadata", __FUNCTION__, mtk_tag);
            continue;
        }

        //(3) get android tag's type
        int android_type = get_camera_metadata_tag_type(android_tag);
        int mtk_type = getTagInfo()->getType(mtk_tag);

        CAM_LOGV("mtk (tag: 0x%x, name: %s, type: %d), android (tag: 0x%x, type: %d), data_count:%d", mtk_tag, getTagInfo()->getName(mtk_tag), mtk_type, android_tag, android_type, rMetadata.entryFor(mtk_tag).count());

        //(4.1) types are equal ==> normal copy.
        if (android_type == mtk_type)
        {
            NORMAL_COPY_TO_ANDROID(mtk_tag, android_tag, pDstMetadata, rMetadata.entryFor(mtk_tag), rMetadata.entryFor(mtk_tag).count());
        }

        //(4.2) types are not equal
        else
        {

            if(mtk_type == TYPE_MRect && android_type == TYPE_INT32) {
                Vector<int32_t> storage;
                MRect2Array(storage, rMetadata.entryFor(mtk_tag));
                if (OK!=update(pDstMetadata, android_tag, &storage[0], rMetadata.entryFor(mtk_tag).count()*4)){
                    return MFALSE;
                }
            }
            else if(mtk_type == TYPE_MSize && android_type == TYPE_INT32)
            {
                Vector<int32_t> storage;
                MSize2Array(storage, rMetadata.entryFor(mtk_tag));
                if (OK!=update(pDstMetadata, android_tag, &storage[0], rMetadata.entryFor(mtk_tag).count()*2)){
                    return MFALSE;
                }
            }
        }
    }

    return MTRUE;
}

// IMetadata --> camera_metadata
// no need to allocate memory
MBOOL
MetadataConverter::
convertWithoutAllocate(const IMetadata &rMetadata, camera_metadata* &pDstMetadata) const
{
    CAM_LOGV("Convert from IMetadata to camera_metadata without allocation");

    size_t entryCount = 0;
    size_t dataCount = 0;
    size_t meta_size = 0;
    if(pDstMetadata == NULL)
    {
        //allocate
        MBOOL ret = get_data_count(rMetadata, entryCount, dataCount);
        if ( ret != OK )
        {
            MY_LOGE("get Imetadata count error\n");
            return MFALSE;
        }
        MY_LOGD("Allocating %d entries, %d extra bytes from HAL modules", entryCount, dataCount);

        pDstMetadata = ::allocate_camera_metadata(entryCount, dataCount);
        if ( !pDstMetadata )
        {
            MY_LOGE(
                "Unable to allocate camera static info (%d entries, %d bytes extra data)\n",
                entryCount, dataCount
            );
            return  MFALSE;
        }
    } else {
        entryCount = ::get_camera_metadata_entry_capacity(pDstMetadata);
        dataCount  = ::get_camera_metadata_data_capacity(pDstMetadata);
        meta_size  = ::get_camera_metadata_size(pDstMetadata);

        pDstMetadata = place_camera_metadata(pDstMetadata,
                                             meta_size,
                                             entryCount,
                                             dataCount);
    }

    if( !tryToConvert(rMetadata, pDstMetadata, entryCount, dataCount) )
    {
        //allocate
        ::free_camera_metadata(pDstMetadata);
        pDstMetadata = NULL;

        pDstMetadata = ::allocate_camera_metadata(entryCount, dataCount);
        if ( !pDstMetadata )
        {
            MY_LOGE(
                "Unable to allocate camera static info (%d entries, %d bytes extra data)\n",
                entryCount, dataCount
            );
            return  MFALSE;
        }

        return tryToConvert(rMetadata, pDstMetadata, entryCount, dataCount);
    }

    return MTRUE;
}

MBOOL
MetadataConverter::
tryToConvert(const IMetadata &rMetadata, camera_metadata* &pDstMetadata, size_t &entryCount, size_t &dataCount) const
{
    size_t current_entryCount = 0;
    size_t current_dataCount = 0;
    //start to convert
    for (uint32_t i = 0; i < rMetadata.count(); i++)
    {
        //(1) get tag by index, and skip it if it doesn't exist in android-metadata-tag list.
        MUINT32 mtk_tag = rMetadata.entryAt(i).tag();

        //(2) map to tag defined by android; return if undefined.
        #if (PLATFORM_SDK_VERSION >= 21)
        MUINT32 android_tag = getTagInfo()->getAndroidTag((MUINT32)mtk_tag);
        #else
        MUINT32 android_tag = 0;
        #endif
        if (android_tag == IMetadata::IEntry::BAD_TAG){
            CAM_LOGV("%s: Tag 0x%x not found in Android Metadata", __FUNCTION__, mtk_tag);
            continue;
        }

        //(3) get android tag's type
        int android_type = get_camera_metadata_tag_type(android_tag);
        int mtk_type = getTagInfo()->getType(mtk_tag);

        CAM_LOGV("mtk (tag: 0x%x, name: %s, type: %d), android (tag: 0x%x, type: %d), data_count:%d", mtk_tag, getTagInfo()->getName(mtk_tag), mtk_type, android_tag, android_type, rMetadata.entryFor(mtk_tag).count());

        //(4.1) types are equal ==> normal copy.
        if (android_type == mtk_type)
        {
            current_entryCount++;
            current_dataCount += ::calculate_camera_metadata_entry_data_size(android_type, rMetadata.entryFor(mtk_tag).count());

            if(current_entryCount <= entryCount && current_dataCount <= dataCount) {
                NORMAL_COPY_TO_ANDROID(mtk_tag, android_tag, pDstMetadata, rMetadata.entryFor(mtk_tag), rMetadata.entryFor(mtk_tag).count());
            }
        }

        //(4.2) types are not equal
        else
        {
            current_entryCount++;

            if(mtk_type == TYPE_MRect && android_type == TYPE_INT32) {
                current_dataCount += ::calculate_camera_metadata_entry_data_size(android_type, rMetadata.entryFor(mtk_tag).count()*4);

                if(current_entryCount <= entryCount && current_dataCount <= dataCount) {
                    Vector<int32_t> storage;
                    MRect2Array(storage, rMetadata.entryFor(mtk_tag));
                    if (OK!=update(pDstMetadata, android_tag, &storage[0], rMetadata.entryFor(mtk_tag).count()*4)){
                        return MFALSE;
                    }
                }
            }
            else if(mtk_type == TYPE_MSize && android_type == TYPE_INT32) {
                current_dataCount += ::calculate_camera_metadata_entry_data_size(android_type, rMetadata.entryFor(mtk_tag).count()*2);

                if(current_entryCount <= entryCount && current_dataCount <= dataCount) {
                    Vector<int32_t> storage;
                    MSize2Array(storage, rMetadata.entryFor(mtk_tag));
                    if (OK!=update(pDstMetadata, android_tag, &storage[0], rMetadata.entryFor(mtk_tag).count()*2)){
                        return MFALSE;
                    }
                }
            }
        }
    }

    if(current_entryCount > entryCount || current_dataCount > dataCount) {
        CAM_LOGD("tryToConvert buffer not enough, original entryCount = %d, dataCount = %d, new entryCount = %d, dataCount = %d", entryCount, dataCount, current_entryCount, current_dataCount);
        entryCount = current_entryCount;
        dataCount = current_dataCount;
        return MFALSE;
    }
    return MTRUE;
}

#undef  ASSIGN_ENTRY_TO_VOID_PTR
#undef  NORMAL_COPY_TO_ANDROID
#undef  SPECIAL_COPY_TO_ANDROID_ON_SAMPLE
#undef  SPECIAL_COPY_TO_ANDROID_ON_TYPE

/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataConverter::
update(camera_metadata* &mBuffer, MUINT32 tag, const void *data, MUINT32 data_count)
{
    //int type = get_camera_metadata_tag_type(tag);
    //size_t data_size = calculate_camera_metadata_entry_data_size(type, data_count);

    //status_t res = resizeIfNeeded(mBuffer, 1, data_size);
    status_t res = OK;

    //add or update
    if (res == OK) {
        camera_metadata_entry_t entry;

        res = find_camera_metadata_entry(mBuffer, tag, &entry);
        if (res == NAME_NOT_FOUND) {
            res = add_camera_metadata_entry(mBuffer,
                    tag, data, data_count);
        } else if (res == OK) {
            res = update_camera_metadata_entry(mBuffer,
                    entry.index, data, data_count, NULL);
        }
    }

    if (res != OK) {
        CAM_LOGE("%s: Unable to update metadata entry %s.%s (%x): %s (%d)",
                __FUNCTION__, get_camera_metadata_section_name(tag),
                get_camera_metadata_tag_name(tag), tag, strerror(-res), res);
        return NO_MEMORY;
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataConverter::
resizeIfNeeded(camera_metadata* &pBuffer, size_t extraEntries, size_t extraData)
{
    size_t currentEntryCount = get_camera_metadata_entry_count(pBuffer);
    size_t currentEntryCap = get_camera_metadata_entry_capacity(pBuffer);
    size_t newEntryCount = currentEntryCount +
            extraEntries;
    newEntryCount = (newEntryCount > currentEntryCap) ?
            newEntryCount * 2 : currentEntryCap;

    size_t currentDataCount = get_camera_metadata_data_count(pBuffer);
    size_t currentDataCap = get_camera_metadata_data_capacity(pBuffer);
    size_t newDataCount = currentDataCount +
            extraData;
    newDataCount = (newDataCount > currentDataCap) ?
            newDataCount * 2 : currentDataCap;

    if (newEntryCount > currentEntryCap ||
            newDataCount > currentDataCap) {
        CAM_LOGW("re-allocate buffer");
        camera_metadata_t *oldBuffer = pBuffer;
        pBuffer = allocate_camera_metadata(newEntryCount,
                newDataCount);
        if (pBuffer == NULL) {
            CAM_LOGE("%s: Can't allocate larger metadata buffer", __FUNCTION__);
            return NO_MEMORY;
        }
        append_camera_metadata(pBuffer, oldBuffer);
        free_camera_metadata(oldBuffer);
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
MetadataConverter::
get_data_count(IMetadata const &halmetadata, size_t &entryCount, size_t &dataCount) const
{
    entryCount = 0;
    dataCount = 0;
    MY_LOGV("get_data_count - count:%d ", halmetadata.count());
    for(size_t i = 0; i < halmetadata.count(); i++)
    {
        uint32_t mtk_tag = halmetadata.entryAt(i).tag();

        MY_LOGV("get_data_count - mtk_tag:%d ", mtk_tag);

        //map to tag defined by android; return if undefined.
        #if (PLATFORM_SDK_VERSION >= 21)
        MUINT32 android_tag = getTagInfo()->getAndroidTag((MUINT32)mtk_tag);
        #else
        MUINT32 android_tag = 0;
        #endif
        if (android_tag == IMetadata::IEntry::BAD_TAG){
            CAM_LOGD("%s: Tag 0x%x not found in Android Metadata", __FUNCTION__, mtk_tag);
            continue;
        }

        CAM_LOGV("0x%x, %s", android_tag, getTagInfo()->getName((MUINT32)mtk_tag));

        int android_type = get_camera_metadata_tag_type(android_tag);
        if  ( android_type < 0 )
        {
            MY_LOGE("android_Typee - tag:%d has no type:%d < 0", android_tag, android_type);
            continue;
            //return  BAD_VALUE;
        }

        int mtk_type = MetadataConverter::getTagInfo()->getType(mtk_tag);
        if  ( mtk_type < 0 )
        {
            MY_LOGE("mtk_typee - tag:%d has no type:%d < 0", mtk_tag, mtk_type);
            continue;
            //return  BAD_VALUE;
        }

        if (android_type == mtk_type)
        {
            entryCount++;
            dataCount += ::calculate_camera_metadata_entry_data_size(android_type, halmetadata.entryAt(i).count());
        }
        else
        {
            entryCount++;

            //special copy, by type.
            if(mtk_type == TYPE_MRect && android_type == TYPE_INT32) {
                dataCount += ::calculate_camera_metadata_entry_data_size(android_type, halmetadata.entryAt(i).count() * 4);
            }

            else if(mtk_type == TYPE_MSize && android_type == TYPE_INT32)
            {
                dataCount += ::calculate_camera_metadata_entry_data_size(android_type, halmetadata.entryAt(i).count() * 2);
            }

        }
    }

    MY_LOGD(
        "Allocating %d entries, %d extra bytes from HAL modules",
        entryCount, dataCount
    );

    return OK;
}

