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


/*******************************************************************************
*
********************************************************************************/

#include <metadata/client/TagMap.h>
#include <metadata/IMetadataTagSet.h>
#include <metadata/IMetadataConverter.h>
#include <system/camera_metadata.h>
#include <Log.h>
#include <metadata/mtk_metadata_types.h>


using namespace android;


/*******************************************************************************
*
********************************************************************************/

#define META_CONVERTER          "{MetaDataConverter} "


#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)

#define FUNCTION_LOG_START      MY_LOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        do {if(!ret) MY_LOGE("[%s] fail", __FUNCTION__); MY_LOGD("[%s] - X. ret=%d", __FUNCTION__, ret);} while(0)
#define FUNCTION_LOG_END_MUM    MY_LOGD("[%s] - X.", __FUNCTION__)
#define ASSERT_IF_NOT_EQAL(_In1, _In2) \
        CAM_LOGA_IF(_In1 != _In2, "should be %d, but is %d", _In2, _In1)

/*******************************************************************************
*
********************************************************************************/

using namespace NSCam;

/*******************************************************************************
*
********************************************************************************/

bool setTagInfo(IMetadataTagSet &rtagInfo);

/*******************************************************************************
*
********************************************************************************/


int main(int, char**)
{
    MBOOL ret = MTRUE;
    FUNCTION_LOG_START;


    //prepare data for TagInfo
    #if 0
    IMetadataTagSet mtagInfo;

    setTagInfo(mtagInfo);
    #else
    //get global metadata tag set
    IMetadataTagSet const &mtagInfo = IDefaultMetadataTagSet::singleton()->getTagSet();
    #endif
    //create IMetadataConverter
    sp<IMetadataConverter> mMetaDataConverter = IMetadataConverter::createInstance(mtagInfo);

    // do converter test

    //construction

    //MRect
    MRect region1(MPoint(2,3),MSize(4,5));
    IMetadata::IEntry scalerCropR(MTK_SCALER_CROP_REGION);
    scalerCropR.push_back(region1, Type2Type< MRect >());

    //MFLOAT
    IMetadata::IEntry colorCorrGain(MTK_COLOR_CORRECTION_GAINS);
    colorCorrGain.push_back(1.5, Type2Type< MFLOAT >());

    //MINT64
    IMetadata::IEntry flashFiringTime(MTK_FLASH_FIRING_TIME);
    flashFiringTime.push_back(333333, Type2Type< MINT64 >());

    //MUINT8
    IMetadata::IEntry blackLevelLock(MTK_BLACK_LEVEL_LOCK);
    blackLevelLock.push_back(100, Type2Type< MUINT8 >());

    IMetadata metadata_MTK;
    metadata_MTK.update(MTK_SCALER_CROP_REGION, scalerCropR);
    metadata_MTK.update(MTK_COLOR_CORRECTION_GAINS, colorCorrGain);
    metadata_MTK.update(MTK_FLASH_FIRING_TIME, flashFiringTime);
    metadata_MTK.update(MTK_BLACK_LEVEL_LOCK, blackLevelLock);



    camera_metadata* metadata_Android;

    //convert from MTK Metadata to Android MetaData
    mMetaDataConverter->convert(metadata_MTK, metadata_Android);

    //convert from Android MetaData to MTK Metadata
    IMetadata metadata_ConvertFromAndroid;

    mMetaDataConverter->convert(metadata_Android, metadata_ConvertFromAndroid);
    mMetaDataConverter->dump(metadata_ConvertFromAndroid);
    mMetaDataConverter->dumpAll(metadata_ConvertFromAndroid);


    //check result
    ASSERT_IF_NOT_EQAL(metadata_ConvertFromAndroid.entryFor(MTK_SCALER_CROP_REGION).itemAt(0, Type2Type< MRect >()).p.x, region1.p.x);
    ASSERT_IF_NOT_EQAL(metadata_ConvertFromAndroid.entryFor(MTK_SCALER_CROP_REGION).itemAt(0, Type2Type< MRect >()).p.y, region1.p.y);
    ASSERT_IF_NOT_EQAL(metadata_ConvertFromAndroid.entryFor(MTK_SCALER_CROP_REGION).itemAt(0, Type2Type< MRect >()).s.w, region1.s.w);
    ASSERT_IF_NOT_EQAL(metadata_ConvertFromAndroid.entryFor(MTK_SCALER_CROP_REGION).itemAt(0, Type2Type< MRect >()).s.h, region1.s.h);
    ASSERT_IF_NOT_EQAL(metadata_ConvertFromAndroid.entryFor(MTK_COLOR_CORRECTION_GAINS).itemAt(0, Type2Type< MFLOAT >()), 1.5);
    ASSERT_IF_NOT_EQAL(metadata_ConvertFromAndroid.entryFor(MTK_FLASH_FIRING_TIME).itemAt(0, Type2Type< MINT64 >()), 333333);
    ASSERT_IF_NOT_EQAL(metadata_ConvertFromAndroid.entryFor(MTK_BLACK_LEVEL_LOCK).itemAt(0, Type2Type< MUINT8 >()), 100);

    //remove
    free_camera_metadata(metadata_Android);


lbExit:
    FUNCTION_LOG_END;
    return  ret;

}

bool setTagInfo(IMetadataTagSet &rtagInfo)
{


    #define _IMP_SECTION_INFO_(...)
    #undef  _IMP_TAG_INFO_
    #define _IMP_TAG_INFO_(_tag_, _type_, _name_) \
        rtagInfo.addTag(_tag_, _name_, Type2TypeEnum<_type_>::typeEnum);
    #include <metadata/client/mtk_metadata_tag_info.inl>
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







