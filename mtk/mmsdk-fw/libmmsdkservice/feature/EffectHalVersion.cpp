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

//#include <cutils/xlog.h>
#include <cutils/log.h>
//#include "HdrEffectHal.h"
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <mmsdk/IEffectHal.h>


/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)


using namespace NSCam;
using namespace android;

size_t
EffectHalVersion::
getFlattenedSize() const
{
    size_t fixedSize =
            sizeof(uint32_t) * 2;

    size_t variableSize =
            sizeof(uint32_t) + FlattenableUtils::align<4>(effectName.length()*sizeof(char16_t));

    return fixedSize + variableSize;
}


status_t
EffectHalVersion::
flatten(void* buffer, size_t size) const
{
#if 1
    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }

    flattenString8(buffer, size, effectName);
    FlattenableUtils::write(buffer, size, major);
    FlattenableUtils::write(buffer, size, minor);
    return NO_ERROR;
#else
    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }

    FlattenableUtils::write(buffer, size, 1);
    FlattenableUtils::write(buffer, size, 0);
    return NO_ERROR;
    flattenString8(buffer, size, String8("Hdr"));
#endif
}


status_t
EffectHalVersion::
unflatten(void const* buffer, size_t size)
{
    if (!unflattenString8(buffer, size, effectName)) {
        return NO_MEMORY;
    }

    size_t fixedSize =
            sizeof(uint32_t) * 2;
    if(size < fixedSize) {
        return NO_MEMORY;
    }
    FlattenableUtils::read(buffer, size, major);
    FlattenableUtils::read(buffer, size, minor);
    return NO_ERROR;
}

void
EffectHalVersion::
flattenString8(void*& buffer, size_t& size, const String8& string8)
{
    #if 0
    uint32_t len = string8.length();
    FlattenableUtils::write(buffer, size, len);
    memcpy(static_cast<char*>(buffer), string8.string(), len);
    FlattenableUtils::advance(buffer, size, FlattenableUtils::align<4>(len));
    #else
    String16 string16(string8);
    uint32_t len = string16.size();
    FlattenableUtils::write(buffer, size, len);

    len *= sizeof(char16_t);
    memcpy(static_cast<char16_t*>(buffer), string16.string(), len);
    FlattenableUtils::advance(buffer, size, FlattenableUtils::align<4>(len));
    #endif
}


bool
EffectHalVersion::
unflattenString8(void const*& buffer, size_t& size, String8& outputString8)
{
    #if 0
    uint32_t len;
    if (size < sizeof(len)) {
        return false;
    }
    FlattenableUtils::read(buffer, size, len);
    if (size < len) {
        return false;
    }
    outputString8.setTo(static_cast<char const*>(buffer), len);
    FlattenableUtils::advance(buffer, size, FlattenableUtils::align<4>(len));
    return true;
    #else
    uint32_t len;
    if (size < sizeof(len)) {
        return false;
    }
    FlattenableUtils::read(buffer, size, len);
    if (size < len) {
        return false;
    }
    outputString8.setTo(static_cast<char const*>(buffer), len);
    FlattenableUtils::advance(buffer, size, FlattenableUtils::align<4>(len));
    return true;
    #endif
}

