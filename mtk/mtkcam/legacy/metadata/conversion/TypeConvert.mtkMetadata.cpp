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
AddToEntry(IMetadata::IEntry &entry, T* data_start)
{
    entry.push_back(*data_start, Type2Type< T >());
}

#define ARRAY_TO_ENTRY(_ENTRY_, _DATA_START_, _TYPE_) \
    if(_TYPE_ == TYPE_BYTE) \
        AddToEntry(_ENTRY_, (MUINT8*)_DATA_START_); \
    if (_TYPE_ == TYPE_INT32) \
        AddToEntry(_ENTRY_, (MINT32*)_DATA_START_); \
    if(_TYPE_ == TYPE_FLOAT) \
        AddToEntry(_ENTRY_, (MFLOAT*)_DATA_START_); \
    if (_TYPE_ == TYPE_INT64) \
        AddToEntry(_ENTRY_, (MINT64*)_DATA_START_); \
    if(_TYPE_ == TYPE_DOUBLE) \
        AddToEntry(_ENTRY_, (MDOUBLE*)_DATA_START_); \
    if (_TYPE_ == TYPE_RATIONAL) \
        AddToEntry(_ENTRY_, (MRational*)_DATA_START_);


// camera_metadata --> IMetadata
MBOOL
MetadataConverter::
convert(const camera_metadata *pMetadata, IMetadata &rDstBuffer) const
{
    CAM_LOGD("Convert from camera_metadata to IMetadata");

    if (pMetadata == NULL) {
        MY_LOGE("camera_metadat has not allocated");
        return false;
    }
    //MY_LOGD("start for loop\n");
    for (unsigned int i = 0; i < get_camera_metadata_entry_count(pMetadata); i++) {

        //(1) get android entry
        camera_metadata_entry android_entry;
        int result;
        //MY_LOGD("for loop(1)\n");
        if ( OK != (result = get_camera_metadata_entry(const_cast<camera_metadata*>(pMetadata), i, &android_entry)))
        {
           MY_LOGE("cannot get metadata entry");
           continue;
        }

        //(2) get mtk tag
        //MY_LOGD("for loop(2)\n");
        if (getTagInfo() == NULL)
        {
            MY_LOGD("get TagConvert fail\n");
            return false;
        }
        #if (PLATFORM_SDK_VERSION >= 21)
        MUINT32 mtk_tag = getTagInfo()->getMtkTag((MUINT32)android_entry.tag);
        #else
        MUINT32 mtk_tag = 0;
        #endif
        if (mtk_tag == IMetadata::IEntry::BAD_TAG){
            CAM_LOGE("%s: Tag 0x%x not found in Mtk Metadata. Shouldn't happened", __FUNCTION__, android_entry.tag);
            continue;
        }
        IMetadata::IEntry MtkEntry(mtk_tag);

        //(3.1) get types
        //MY_LOGD("for loop(3.1)\n");
        int android_type = android_entry.type;
        int mtk_type = getTagInfo()->getType(mtk_tag);

        CAM_LOGV(" android (tag: 0x%x, type: %d), data count: %d, mtk (tag: 0x%x, name: %s, type: %d)", android_entry.tag, android_type, android_entry.count, mtk_tag, getTagInfo()->getName(mtk_tag), mtk_type);

        //(3.2) types are equal ==> normal copy.
        if (android_type == mtk_type)
        {
            for (unsigned int j = 0; j < android_entry.count * camera_metadata_type_size[android_entry.type]; j+=camera_metadata_type_size[android_entry.type]) {
                ARRAY_TO_ENTRY(MtkEntry, &android_entry.data.u8[j], android_entry.type);
            }
        }

        //(3.3) types are not equal
        else
        {
            //(3.3.1) by sample. priority is higher than by type
            /* this special case shouldn't exist in the latest verson*/
            /*
            if (mtk_tag == MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION &&
                android_entry.tag == ANDROID_SENSOR_INFO_ACTIVE_ARRAY_SIZE){
                MRect data(MPoint(), MSize(android_entry.data.i32[0], android_entry.data.i32[1]));
                AddToEntry(MtkEntry, (MRect*)&data);
            }
            */
            //(3.3.2) by case
            if(android_type == TYPE_INT32 && mtk_type == TYPE_MRect) {
                for(size_t j = 0; j < android_entry.count; j+=4) {
                    MRect data(MPoint(android_entry.data.i32[j*4], android_entry.data.i32[j*4+1]), MSize(android_entry.data.i32[j*4+2], android_entry.data.i32[j*4+3]));
                    AddToEntry(MtkEntry, &data);
                }
            }
            else if(android_type == TYPE_INT32 && mtk_type == TYPE_MSize) {
                for(size_t j = 0; j < android_entry.count; j+=2) {
                    MSize data(android_entry.data.i32[j*2], android_entry.data.i32[j*2+1]);
                    AddToEntry(MtkEntry, &data);
                }
            }
        }

        rDstBuffer.update(mtk_tag, MtkEntry);
    }

    return true;
}

