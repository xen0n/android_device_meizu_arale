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
#define LOG_TAG "mmsdk/BasicParameters"
//#include <cutils/xlog.h>
#include <cutils/log.h>
#include <common.h>
#include <cutils/properties.h>

//#include "HdrEffectHal.h"
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <mmsdk/BasicParameters.h>


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

// #define FUNCTION_LOG_START          MY_LOGD_IF(1<=mLogLevel, "+");
// #define FUNCTION_LOG_END            MY_LOGD_IF(1<=mLogLevel, "-");
#define FUNCTION_LOG_START          MY_LOGD_IF(1<=0, "+");
#define FUNCTION_LOG_END            MY_LOGD_IF(1<=0, "-");
/******************************************************************************
 *
 ******************************************************************************/



using namespace NSCam;
using namespace android;

const char BasicParameters::KEY_PICTURE_SIZE[] = "picture-size";
const char BasicParameters::KEY_PICTURE_FORMAT[] = "picture-format";
const char BasicParameters::KEY_JPEG_THUMBNAIL_SIZES[] = "jpeg-thumbnail-size";
const char BasicParameters::KEY_JPEG_THUMBNAIL_QUALITY[] = "jpeg-thumbnail-quality";
const char BasicParameters::KEY_JPEG_QUALITY[] = "jpeg-quality";
const char BasicParameters::KEY_ROTATION[] = "rotation";
const char BasicParameters::KEY_ZOOM_RATIOS[] = "zoom-ratios";
const char BasicParameters::PIXEL_FORMAT_YUV422SP[] = "yuv422sp";
const char BasicParameters::PIXEL_FORMAT_YUV420SP[] = "yuv420sp";
const char BasicParameters::PIXEL_FORMAT_YUV422I[] = "yuv422i-yuyv";
const char BasicParameters::PIXEL_FORMAT_YUV420P[]  = "yuv420p";
const char BasicParameters::PIXEL_FORMAT_RGB565[] = "rgb565";
const char BasicParameters::PIXEL_FORMAT_RGBA8888[] = "rgba8888";
const char BasicParameters::PIXEL_FORMAT_JPEG[] = "jpeg";

const char BasicParameters::KEY_IMAGE_WIDTH[] = "image-width";
const char BasicParameters::KEY_IMAGE_HEIGHT[] = "image-height";

// Face effect use
// result
const char BasicParameters::KEY_DETECTED_FACE_NUM[] = "effect-face-num";
const char BasicParameters::KEY_DETECTED_FACE_RESULT[] = "effect-face-result";
const char BasicParameters::KEY_DETECTED_SMILE_RESULT[] = "effect-smile-result";
const char BasicParameters::KEY_DETECTED_GESTURE_RESULT[] = "effect-gesture-num";
const char BasicParameters::KEY_DETECTED_SCENE[] = "effect-detected-scene";
// control
const char BasicParameters::KEY_DETECTED_SD_ENABLE[] = "effect-smile-en";
const char BasicParameters::KEY_DETECTED_FD_ENABLE[] = "effect-face-en";
const char BasicParameters::KEY_DETECTED_GS_ENABLE[] = "effect-gesture-en";
const char BasicParameters::KEY_DETECTED_ASD_ENABLE[] = "effect-auto-scene-en";
const char BasicParameters::KEY_DETECTED_ASD_3A_INFO[] = "effect-asd-3a-info";
const char BasicParameters::KEY_DETECTED_USE_FD[] = "effect-use-face-detect";
const char BasicParameters::KEY_DETECTED_USE_GS[] = "effect-use-gesture-detect";
const char BasicParameters::KEY_DETECTED_USE_ASD[] = "effect-use-auto-scene-detect";

static
void
flattenString8(void*& buffer, size_t& size, const String8& string8)
{
    MY_LOGD(" +");
    String16 string16(string8);
    uint32_t len = string16.size();
    FlattenableUtils::write(buffer, size, len);

    len = (len+1)*sizeof(char16_t);
    memcpy(static_cast<char16_t*>(buffer), string16.string(), len);
    FlattenableUtils::advance(buffer, size, FlattenableUtils::align<4>(len));
    MY_LOGD(" -");
}


static
bool
unflattenString8(void const*& buffer, size_t& size, String8& outputString8)
{
    MY_LOGD(" +");
    uint32_t len;
    if (size < sizeof(len)) {
        ALOGE("no enough space");
        return false;
    }
    FlattenableUtils::read(buffer, size, len);
    if (size < len) {
        ALOGE("no enough space");
        return false;
    }
    String16 string16;
    string16.setTo(static_cast<const char16_t *>(buffer), len);
    FlattenableUtils::advance(buffer, size, FlattenableUtils::align<4>((len+1)*sizeof(char16_t)));
    MY_LOGD(" -");
    return true;
}


//-----------------------------------------------------------------------------
//public:     //LightFlattenable
//-----------------------------------------------------------------------------
size_t
BasicParameters::
getFlattenedSize() const
{
    //FUNCTION_LOG_START;
    String16 string16(flatten());
    size_t variableSize =
            sizeof(uint32_t) + FlattenableUtils::align<4>((string16.size()+1)*sizeof(char16_t));
    //FUNCTION_LOG_END;
    return variableSize;
}


status_t
BasicParameters::
flatten(void* buffer, size_t size) const
{
    FUNCTION_LOG_START;
    if (size < getFlattenedSize()) {
        return NO_MEMORY;
    }

    flattenString8(buffer, size, flatten());

    FUNCTION_LOG_END;
    return OK;
}

status_t
BasicParameters::
unflatten(void const* buffer, size_t size)
{
    FUNCTION_LOG_START;
    String8 string8;
    if (!unflattenString8(buffer, size, string8)) {
        return NO_MEMORY;
    }
    unflatten(string8);
    FUNCTION_LOG_END;
    return OK;
}


//-----------------------------------------------------------------------------
//public:     //BasicParameters
//-----------------------------------------------------------------------------
BasicParameters::
BasicParameters() : mMap()
{
    // char cLogLevel[PROPERTY_VALUE_MAX];
    // ::property_get("debug.camera.log", cLogLevel, "0");
    // mLogLevel = ::atoi(cLogLevel);
    // if ( 0 == mLogLevel ) {
    //     ::property_get("debug.camera.log.basicParameter", cLogLevel, "0");
    //     mLogLevel = ::atoi(cLogLevel);
    // }

    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
}

//BasicParameters::
//BasicParameters(const BasicParameters& other) :mMap(other.mMap) {
//}
BasicParameters::BasicParameters(
        const BasicParameters& other) :
        mMap(other.mMap) {
}

String8
BasicParameters::
flatten() const
{
    FUNCTION_LOG_START;

    String8 flattened("");
    size_t size = mMap.size();

    for (size_t i = 0; i < size; i++) {
        String8 k, v;
        k = mMap.keyAt(i);
        v = mMap.valueAt(i);

        flattened += k;
        flattened += "=";
        flattened += v;
        if (i != size-1)
            flattened += ";";
    }
    FUNCTION_LOG_END;

    return flattened;
}


void
BasicParameters::
unflatten(const String8 &params)
{
    FUNCTION_LOG_START;
    const char *a = params.string();
    const char *b;

    mMap.clear();

    for (;;) {
        // Find the bounds of the key name.
        b = strchr(a, '=');
        if (b == 0)
            break;

        // Create the key string.
        String8 k(a, (size_t)(b-a));

        // Find the value.
        a = b+1;
        b = strchr(a, ';');
        if (b == 0) {
            // If there's no semicolon, this is the last item.
            String8 v(a);
            mMap.add(k, v);
            break;
        }

        String8 v(a, (size_t)(b-a));
        mMap.add(k, v);
        a = b+1;
    }
    FUNCTION_LOG_END;
}


void
BasicParameters::
set(const char *key, const char *value)
{
    FUNCTION_LOG_START;
    // XXX i think i can do this with strspn()
    if (strchr(key, '=') || strchr(key, ';')) {
        //XXX ALOGE("Key \"%s\"contains invalid character (= or ;)", key);
        return;
    }

    if (strchr(value, '=') || strchr(value, ';')) {
        //XXX ALOGE("Value \"%s\"contains invalid character (= or ;)", value);
        return;
    }

    mMap.replaceValueFor(String8(key), String8(value));
    FUNCTION_LOG_END;
}


void
BasicParameters::
set(const char *key, int value)
{
    FUNCTION_LOG_START;
    char str[16];
    sprintf(str, "%d", value);
    set(key, str);
    FUNCTION_LOG_END;
}

void
BasicParameters::
setInt64(const char *key, int64_t value)
{
    FUNCTION_LOG_START;
    char str[32];
    sprintf(str, "%" PRId64 "", value);

    ALOGD("[%s] key=%s, v=%s, %" PRId64 "", __FUNCTION__, key, str, value);
    set(key, str);
    FUNCTION_LOG_END;
}

void
BasicParameters::
setPtr(const char *key, void *value)
{
    FUNCTION_LOG_START;
    char str[32];
    MINTPTR val = (MINTPTR)value;
    sprintf(str, "%#" PRIxPTR "", val);

    set(key, str);
    FUNCTION_LOG_END;
}


void
BasicParameters::
setFloat(const char *key, float value)
{
    FUNCTION_LOG_START;
    char str[16];  // 14 should be enough. We overestimate to be safe.
    snprintf(str, sizeof(str), "%g", value);
    set(key, str);
    FUNCTION_LOG_END;
}


const char *
BasicParameters::
get(const char *key) const
{
    FUNCTION_LOG_START;
    String8 v = mMap.valueFor(String8(key));
    if (v.length() == 0)
        return 0;
    FUNCTION_LOG_END;
    return v.string();
}


int64_t
BasicParameters::
getInt64(const char *key) const
{
    FUNCTION_LOG_START;
    const char *v = get(key);
    if (v == 0)
        return -1;
    FUNCTION_LOG_END;
    return strtoll(v, 0, 0);
}

int
BasicParameters::
getInt(const char *key) const
{
    FUNCTION_LOG_START;
    const char *v = get(key);
    if (v == 0)
        return -1;
    FUNCTION_LOG_END;
    return strtol(v, 0, 0);
}

float
BasicParameters::
getFloat(const char *key) const
{
    FUNCTION_LOG_START;
    const char *v = get(key);
    if (v == 0) return -1;
    FUNCTION_LOG_END;

    return strtof(v, 0);
}

void*
BasicParameters::
getPtr(const char *key) const
{
    FUNCTION_LOG_START;
    const char *v = get(key);
    MINTPTR addr;
    if (v == 0)
        return NULL;
    if(strlen(v) > 8) {
        //64bit
        addr = strtoll(v, 0, 16);
    } else {
        //32bit
        addr = strtol(v, 0, 16);
    }
    FUNCTION_LOG_END;

    return (void *)addr;
}


void
BasicParameters::
remove(const char *key)
{
    FUNCTION_LOG_START;
    mMap.removeItem(String8(key));
    FUNCTION_LOG_END;
}

Vector<String8>
BasicParameters::
getKeys()
{
    FUNCTION_LOG_START;
    size_t size = mMap.size();
    Vector<String8> re;
    re.clear();
    re.setCapacity(size);

    for (size_t i = 0; i < size; i++)
    {
        String8 k, v;
        re.push_back(mMap.keyAt(i));
    }

    FUNCTION_LOG_END;
    return re;
}

void
BasicParameters::
dump()
{
    ALOGD("dump: mMap.size = %zu", mMap.size());
    for (size_t i = 0; i < mMap.size(); i++) {
        String8 k, v;
        k = mMap.keyAt(i);
        v = mMap.valueAt(i);
        ALOGD("%s: %s\n", k.string(), v.string());
    }
}

// Parse string like "640x480" or "10000,20000"
int
BasicParameters::
parse_pair(const char *str, int *first, int *second, char delim, char **endptr)
{
    // Find the first integer.
    char *end;
    int w = (int)strtol(str, &end, 10);
    // If a delimeter does not immediately follow, give up.
    if (*end != delim) {
        ALOGE("Cannot find delimeter (%c) in str=%s", delim, str);
        return -1;
    }

    // Find the second integer, immediately after the delimeter.
    int h = (int)strtol(end+1, &end, 10);

    *first = w;
    *second = h;

    if (endptr) {
        *endptr = end;
    }

    return 0;
}
