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

#include <system/camera_metadata.h>
#include <metadata/client/mtk_metadata_tag.h>

//#define LOG_NDEBUG 0
#define LOG_TAG "MTKVendorTags"
#include <cutils/log.h>

#define ATRACE_TAG (ATRACE_TAG_CAMERA | ATRACE_TAG_HAL)
#include <utils/Trace.h>

#include "MTKVendorTags.h"
#include "MyUtils.h"

using namespace NSCam;

namespace NSCam {

// Internal representations of vendor tags for convenience.
// Other classes must access this data via public interfaces.
// Structured to be easy to extend and contain complexity.
namespace {
// Describes a single vendor tag entry
struct Entry {
    const char* name;
    uint8_t     type;
};
// Describes a vendor tag section
struct Section {
    const char* name;
    uint32_t start;
    uint32_t end;
    const Entry* tags;
};

// Entry arrays for each section
const Entry MTKFaceFeature[MTK_FACE_FEATURE_END - MTK_FACE_FEATURE_START] = {
    [MTK_FACE_FEATURE_GESTURE_MODE - MTK_FACE_FEATURE_START] =
        {"gesturemode",   TYPE_INT32},
    [MTK_FACE_FEATURE_GESTURE_RESULT - MTK_FACE_FEATURE_START] =
        {"gestureresult",   TYPE_INT32},
    [MTK_FACE_FEATURE_SMILE_DETECT_MODE - MTK_FACE_FEATURE_START] =
        {"smiledetectmode",   TYPE_INT32},
    [MTK_FACE_FEATURE_SMILE_DETECT_RESULT - MTK_FACE_FEATURE_START] =
        {"smiledetectresult",   TYPE_INT32},
    [MTK_FACE_FEATURE_ASD_MODE - MTK_FACE_FEATURE_START] =
        {"asdmode",   TYPE_INT32},
    [MTK_FACE_FEATURE_ASD_RESULT - MTK_FACE_FEATURE_START] =
        {"asdresult",   TYPE_INT32},
    [MTK_FACE_FEATURE_AVAILABLE_GESTURE_MODES - MTK_FACE_FEATURE_START] =
        {"availablegesturemodes",   TYPE_INT32},
    [MTK_FACE_FEATURE_AVAILABLE_SMILE_DETECT_MODES - MTK_FACE_FEATURE_START] =
        {"availablesmiledetectmodes",   TYPE_INT32},
    [MTK_FACE_FEATURE_AVAILABLE_ASD_MODES - MTK_FACE_FEATURE_START] =
        {"availableasdmodes",   TYPE_INT32}
};

const Entry MTKNRFeature[MTK_NR_FEATURE_END - MTK_NR_FEATURE_START] = {
    [MTK_NR_FEATURE_3DNR_MODE - MTK_NR_FEATURE_START] =
        {"3dnrmode",   TYPE_INT32},
    [MTK_NR_FEATURE_AVAILABLE_3DNR_MODES - MTK_NR_FEATURE_START] =
        {"available3dnrmodes",   TYPE_INT32}
};

// Array of all sections
const Section MTKSections[MTK_VENDOR_SECTION_COUNT] = {
    [MTK_FACE_FEATURE] = { "com.mediatek.facefeature",
                        (uint32_t)MTK_FACE_FEATURE_START,
                        (uint32_t)MTK_FACE_FEATURE_END,
                        MTKFaceFeature },
    [MTK_NR_FEATURE] = { "com.mediatek.nrfeature",
                        (uint32_t)MTK_NR_FEATURE_START,
                        (uint32_t)MTK_NR_FEATURE_END,
                        MTKNRFeature },
};

// Get a static handle to a specific vendor tag section
const Section* getSection(uint32_t tag)
{
    uint32_t section = (tag - MTK_VENDOR_TAG_SECTION_START) >> 16;

    if (tag < (uint32_t)MTK_VENDOR_TAG_SECTION_START) {
        ALOGE("%s: Tag 0x%x before vendor section", __func__, tag);
        return NULL;
    }

    if (section >= MTK_VENDOR_SECTION_COUNT) {
        ALOGE("%s: Tag 0x%x after vendor section", __func__, tag);
        return NULL;
    }

    return &MTKSections[section];
}

// Get a static handle to a specific vendor tag entry
const Entry* getEntry(uint32_t tag)
{
    const Section* section = getSection(tag);
    uint32_t index;

    if (section == NULL)
        return NULL;

    if (tag >= section->end) {
        ALOGE("%s: Tag 0x%x outside section", __func__, tag);
        return NULL;
    }

    index = tag - section->start;
    return &section->tags[index];
}
} // namespace
} // NSCam


MTKVendorTags::MTKVendorTags()
  : mTagCount(0)
{
    for (int i = 0; i < MTK_VENDOR_SECTION_COUNT; i++) {
        mTagCount += MTKSections[i].end - MTKSections[i].start;
    }
}

MTKVendorTags::~MTKVendorTags()
{
}

int MTKVendorTags::getTagCount(const vendor_tag_ops_t* /*ops*/)
{
    return mTagCount;
}

void MTKVendorTags::getAllTags(const vendor_tag_ops_t* /*ops*/, uint32_t* tag_array)
{
    if (tag_array == NULL) {
        ALOGE("%s: NULL tag_array", __func__);
        return;
    }

    ALOGV("getAllTags");

    for (int i = 0; i < MTK_VENDOR_SECTION_COUNT; i++) {
        for (uint32_t tag = MTKSections[i].start;
                tag < MTKSections[i].end; tag++) {
            *tag_array++ = tag;
        }
    }
}

const char* MTKVendorTags::getSectionName(const vendor_tag_ops_t* /*ops*/, uint32_t tag)
{
    const Section* section = getSection(tag);

    if (section == NULL)
        return NULL;

    ALOGV("getSectionName : %s", section->name);

    return section->name;
}

const char* MTKVendorTags::getTagName(const vendor_tag_ops_t* /*ops*/, uint32_t tag)
{
    const Entry* entry = getEntry(tag);

    if (entry == NULL)
        return NULL;

    ALOGV("getTagName : %s", entry->name);

    return entry->name;
}

int MTKVendorTags::getTagType(const vendor_tag_ops_t* /*ops*/, uint32_t tag)
{
    const Entry* entry = getEntry(tag);

    if (entry == NULL)
        return -1;

    ALOGV("getTagType : %d", entry->type);

    return entry->type;
}

extern "C"
NSCam::MTKVendorTags*
getMTKVendorTagsSingleton()
{
    static NSCam::MTKVendorTags inst;
    return &inst;
}

extern "C" {
static int get_tag_count(const vendor_tag_ops_t* ops)
{
    return getMTKVendorTagsSingleton()->getTagCount(ops);
}

static void get_all_tags(const vendor_tag_ops_t* ops, uint32_t* tag_array)
{
    getMTKVendorTagsSingleton()->getAllTags(ops, tag_array);
}

static const char* get_section_name(const vendor_tag_ops_t* ops, uint32_t tag)
{
    return getMTKVendorTagsSingleton()->getSectionName(ops, tag);
}

static const char* get_tag_name(const vendor_tag_ops_t* ops, uint32_t tag)
{
    return getMTKVendorTagsSingleton()->getTagName(ops, tag);
}

static int get_tag_type(const vendor_tag_ops_t* ops, uint32_t tag)
{
    return getMTKVendorTagsSingleton()->getTagType(ops, tag);
}
}

void MTKVendorTags::getVendorTagOps(vendor_tag_ops_t* ops)
{
    ops->get_tag_count      = get_tag_count;
    ops->get_all_tags       = get_all_tags;
    ops->get_section_name   = get_section_name;
    ops->get_tag_name       = get_tag_name;
    ops->get_tag_type       = get_tag_type;
}
