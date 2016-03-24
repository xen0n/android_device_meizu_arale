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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_V3_TEMPLATE_REQUEST_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_V3_TEMPLATE_REQUEST_H_
//
#include <utils/RefBase.h>
#include <system/camera_metadata.h>
//
#include <mtkcam/common.h>
#include <mtkcam/metadata/ITemplateRequest.h>
#include <mtkcam/metadata/IMetadataConverter.h>
#include "custom/Info.h"
#include <utils/KeyedVector.h>
#include <utils/String8.h>

using namespace NSCam;

/******************************************************************************
 *
 ******************************************************************************/
namespace android {
namespace NSTemplateRequest {

/******************************************************************************
 *
 ******************************************************************************/
class TemplateRequest : public ITemplateRequest
{

public:
                                    TemplateRequest() {}

    virtual                         ~TemplateRequest() {}

    virtual camera_metadata const*  getData(int requestType);
    virtual IMetadata const&        getMtkData(int requestType);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Definitions.
    typedef DefaultKeyedVector<int, camera_metadata*>   RequestTemplateMap_t;
    typedef DefaultKeyedVector<int, IMetadata>          RequestTemplateMetadataMap_t;

protected:  ////                    Data Members.
    Info                            mInfo;
    RequestTemplateMap_t            mMapRequestTemplate;
    RequestTemplateMetadataMap_t    mMapRequestTemplateMetadata;

protected:  ////                    Operations (Request Metadata).
    virtual status_t                constructRequestMetadata(
                                        int const           requestType,
                                        camera_metadata*&   rpMetadata,
                                        IMetadata&          rMtkMetadata
                                    );
    virtual status_t                impConstructRequestMetadata(
                                        IMetadata&          metadata,
                                        int const           requestType
                                    );
    virtual status_t                impConstructRequestMetadata_by_SymbolName(
                                        String8 const&      s8Symbol,
                                        IMetadata&          metadata,
                                        int const           requestType
                                    );

    virtual void                    updateData(IMetadata &rMetadata);

public:
    virtual status_t                onCreate(int iOpenId);
};

/******************************************************************************
 *
 ******************************************************************************/
};
};

#endif  //_MTK_PLATFORM_HARDWARE_MTKCAM_V3_TEMPLATE_REQUEST_H_

