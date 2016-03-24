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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_UTILS_V3_METADATAPROVIDER_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_UTILS_V3_METADATAPROVIDER_H_
//

#include <system/camera_metadata.h>
#include <mtkcam/metadata/IMetadata.h>
//
#include <mtkcam/common.h>
#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/metadata/IMetadataConverter.h>
#include "custom/Info.h"
#include <utils/KeyedVector.h>
#include <utils/String8.h>

using namespace NSCam;
/******************************************************************************
 *
 ******************************************************************************/
namespace android {
namespace NSMetadataProvider {


/******************************************************************************
 *  Metadata Provider
 ******************************************************************************/
class MetadataProvider : public IMetadataProvider
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    Info                            mInfo;
    camera_metadata*                mpStaticCharacteristics;
    IMetadata                       mpHALMetadata;

public:     ////                    Instantiation.
    virtual                         ~MetadataProvider();
                                    MetadataProvider(
                                        int32_t const   i4OpenId
                                    );


    virtual uint32_t                getDeviceVersion() const;
    virtual int                     getDeviceFacing() const;
    virtual int                     getDeviceWantedOrientation() const;
    virtual int                     getDeviceSetupOrientation() const;
    virtual int                     getDeviceHasFlashLight() const;

protected:  ////                    Operations (Static Metadata).
    virtual status_t                constructStaticMetadata(
                                        sp<IMetadataConverter> pConverter,
                                        camera_metadata*&   rpMetadata,
                                        IMetadata&          mtkMetadata
                                    );
    virtual status_t                impConstructStaticMetadata(
                                        IMetadata &metadata
                                    );
    virtual status_t                impConstructStaticMetadata_by_SymbolName(
                                        String8 const&      s8Symbol,
                                        IMetadata &metadata
                                    );

            void                    updateData(IMetadata &rMetadata);


public:     ////                    Operations.
    virtual status_t                onCreate();

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////                    Static Info Interface.

    virtual camera_metadata const*  getStaticCharacteristics()  const   { return mpStaticCharacteristics; }
    virtual IMetadata const&        geMtktStaticCharacteristics() const { return mpHALMetadata; }
};


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSMetadataProvider
};  // namespace android
#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_UTILS_V3_METADATAPROVIDER_H_

