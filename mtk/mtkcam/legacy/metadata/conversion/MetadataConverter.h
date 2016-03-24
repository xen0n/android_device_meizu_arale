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

#ifndef _MTK_HARDWARE_MTKCAM_METADATA_CONVERSION_METADATACONVERTER_H_
#define _MTK_HARDWARE_MTKCAM_METADATA_CONVERSION_METADATACONVERTER_H_
//

#include <system/camera_metadata.h>
#include <metadata/IMetadata.h>
//
#include <common.h>
#include <metadata/IMetadataConverter.h>
#include <utils/KeyedVector.h>
#include <utils/String8.h>
using namespace NSCam;
/******************************************************************************
 *
 ******************************************************************************/
namespace android {

/******************************************************************************
 *  Metadata Converter
 ******************************************************************************/
class MetadataConverter : public IMetadataConverter
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    IMetadataTagSet             mpTagInfo;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:  ////
                                MetadataConverter(IMetadataTagSet const &pTagInfo);
                                ~MetadataConverter(){};

    IMetadataTagSet const*      getTagInfo() const  { return &mpTagInfo; }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// MTKMetadata
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    MBOOL                       convert(const camera_metadata *, IMetadata &) const;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AndroidMetadata
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    MBOOL                       convert(const IMetadata &, camera_metadata* &) const;
    MBOOL                       convertWithoutAllocate(const IMetadata &, camera_metadata* &) const;

    status_t                    get_data_count(IMetadata const&, size_t &, size_t &) const;

    MVOID                       dump(const IMetadata &rMetadata, int frameNo) const;
    MVOID                       dumpAll(const IMetadata &rMetadata, int frameNo)const ;

private:
    MBOOL                       tryToConvert(const IMetadata &rMetadata,
                                             camera_metadata* &pDstMetadata,
                                             size_t &entryCount,
                                             size_t &dataCount) const;

    static status_t             update(camera_metadata* &mBuffer,
                                       MUINT32 tag, const void *data,
                                       MUINT32 data_count);

    static status_t             resizeIfNeeded(camera_metadata* &pBuffer,
                                               size_t extraEntries,
                                               size_t extraData);

    MVOID                       print(const IMetadata &rMetadata, const MUINT32 tag, const MUINT32 type) const;
};



/******************************************************************************
 *
 ******************************************************************************/
};  // namespace android
#endif  //_MTK_HARDWARE_MTKCAM_METADATA_CONVERSION_METADATACONVERTER_H_

