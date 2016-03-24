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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_IMETADATAPROVIDER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_IMETADATAPROVIDER_H_
//
#include <utils/RefBase.h>
#include "IMetadata.h"

struct camera_metadata;

namespace NSCam {
/******************************************************************************
 *
 ******************************************************************************/


/******************************************************************************
 *  Metadata Provider Interface
 ******************************************************************************/
class IMetadataProvider : public android::RefBase
{

public:     ////                    Creation.
    static  IMetadataProvider*              create(
                                                int32_t const   i4OpenId
                                            );

public:     ////                    Instantiation.
    virtual                                 ~IMetadataProvider() {}

public:     ////                    Interface.

    virtual camera_metadata const*          getStaticCharacteristics() const   = 0;

    virtual IMetadata const&                geMtktStaticCharacteristics() const = 0;

    virtual uint32_t                        getDeviceVersion() const = 0;

    virtual int                             getDeviceFacing() const = 0;

    virtual int                             getDeviceWantedOrientation() const = 0;

    virtual int                             getDeviceSetupOrientation() const = 0;

    virtual int                             getDeviceHasFlashLight() const = 0;
};


/******************************************************************************
 *  MetadataProvider Manager
 ******************************************************************************/
namespace NSMetadataProviderManager
{
    void                            clear();
    ssize_t                         add(int32_t deviceId, IMetadataProvider* pProvider);

    IMetadataProvider*              valueFor(int32_t deviceId);
    IMetadataProvider*              valueAt(size_t index);
    int32_t                         keyAt(size_t index);
    ssize_t                         indexOfKey(int32_t deviceId);

};  //  NSMetadataProviderManager


/******************************************************************************
 *
 ******************************************************************************/
};  //  NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_IMETADATAPROVIDER_H_

