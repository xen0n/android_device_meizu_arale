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

#define LOG_TAG "mmutils/GrallocHelper"
//
#include "../inc/Log.h"
#include <IGrallocHelper.h>
//
#include <cutils/properties.h>
//
#include <utils/String8.h>
#include <utils/Mutex.h>
#include <utils/Timers.h>
#include <utils/KeyedVector.h>
//
#include <ui/GraphicBufferMapper.h>
#include <ui/GraphicBuffer.h>
#include <ui/gralloc_extra.h>
//
using namespace android;
using namespace mmutils;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        MM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        MM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        MM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        MM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        MM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        MM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        MM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
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
#define GRALLOC_ALIGN( value, base ) (((value) + ((base) - 1)) & ~((base) - 1))
#define ADDRESS_OFFSET(base1, base2) (intptr_t( base1 ) - intptr_t( base2 ))


/******************************************************************************
 *
 ******************************************************************************/
namespace {
// Use "adb shell setprop debug.mmutils.gralloc.loglevel 1" to change the log level.
static volatile int32_t gLogLevel = 0;

static void setLogLevel(int level)
{
    ::android_atomic_write(level, &gLogLevel);
}

};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace {
/**
 *
 */
String8
gQueryPixelFormatName(int format)
{
#define _ENUM_TO_NAME_(_prefix_, _format_) \
    case _prefix_##_format_: \
        {static String8 name(#_format_); return name;}break

    switch  (format)
    {
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGBA_8888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGBX_8888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGB_888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RGB_565);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, BGRA_8888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YV12);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, Y8);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, Y16);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, BLOB);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, IMPLEMENTATION_DEFINED);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCbCr_420_888);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCbCr_422_SP);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCrCb_420_SP);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, YCbCr_422_I);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RAW16);
    _ENUM_TO_NAME_(HAL_PIXEL_FORMAT_, RAW_OPAQUE);
    default:
        break;
    }

#undef  _ENUM_TO_NAME_
    //
    return  String8::empty();
}


/**
 *
 */
String8
gQueryGrallocUsageName(int usage)
{
#define _USAGE_TO_NAME_(_prefix_, _usage_) \
    case _prefix_##_usage_:{str += "|"#_usage_;}break

    String8 str("0");
    //
    switch  ( (usage & GRALLOC_USAGE_SW_READ_MASK) )
    {
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_READ_RARELY);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_READ_OFTEN);
    default: break;
    }
    //
    switch  ( (usage & GRALLOC_USAGE_SW_WRITE_MASK) )
    {
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_WRITE_RARELY);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, SW_WRITE_OFTEN);
    default: break;
    }
    //
    switch  ( (usage & GRALLOC_USAGE_HW_CAMERA_MASK) )
    {
    _USAGE_TO_NAME_(GRALLOC_USAGE_, HW_CAMERA_WRITE);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, HW_CAMERA_READ);
    _USAGE_TO_NAME_(GRALLOC_USAGE_, HW_CAMERA_ZSL);
    default: break;
    }
    //
    if  ( (usage & GRALLOC_USAGE_HW_MASK) != 0 )
    {
#define _USAGE_TO_NAME_OR_(_prefix_, _usage_) \
        if  ( (usage & _prefix_##_usage_) ) {str += "|"#_usage_;}

        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_TEXTURE);
        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_RENDER);
        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_2D);
        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_COMPOSER);
        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_FB);
        _USAGE_TO_NAME_OR_(GRALLOC_USAGE_, HW_VIDEO_ENCODER);

#undef  _USAGE_TO_NAME_OR_
    }
    //
#undef  _USAGE_TO_NAME_
    return  str;
}


/**
 *
 */
status_t
gQueryYCbCr(
    android_ycbcr& ycbcr,
    buffer_handle_t const bh,
    size_t w,
    size_t h
)
{
    status_t status = GraphicBufferMapper::get().lockYCbCr(
        bh, GRALLOC_USAGE_SW_READ_OFTEN,
        Rect((int32_t)w, (int32_t)h),
        &ycbcr
    );
    GraphicBufferMapper::get().unlock(bh);
    //
    ssize_t const offset_cr_y = ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
    ssize_t const offset_cb_cr= ADDRESS_OFFSET(ycbcr.cb, ycbcr.cr);
    MY_LOGD_IF(
        (0<gLogLevel),
        "cr-y:%zu cb-cr:%zu y:%p cb:%p cr:%p ystride:%zu cstride:%zu chroma_step:%zu status:%d",
        offset_cr_y, offset_cb_cr,
        ycbcr.y, ycbcr.cb, ycbcr.cr,
        ycbcr.ystride, ycbcr.cstride,
        ycbcr.chroma_step,
        status
    );
    MY_LOGF_IF(OK != status, "status:%d", status);
    return status;
}
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace {
/**
 *
 */
struct MyStaticInfo : public GrallocStaticInfo
{
    nsecs_t             grallocTime;
    int                 usage;
    size_t              allocSize;
                        //
                        MyStaticInfo()
                        {
                            format = 0;
                            grallocTime = 0;
                            usage = 0;
                            widthInPixels = 0;
                            heightInPixels = 0;
                            allocSize = 0;
                        }
};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace {
/**
 *
 */
inline
bool
checkPlaneSize(
    MyStaticInfo const& info,
    size_t const index,
    size_t const rowNum
)
{
    MyStaticInfo::Plane const& plane = info.planes[index];
    //
    bool isEqual = plane.sizeInBytes == plane.rowStrideInBytes * rowNum;
    MY_LOGW_IF(
        (! isEqual) && (0<gLogLevel),
        "[%s:%zu] sizeInBytes:%zu != %zu x %zu",
        gQueryPixelFormatName(info.format).string(),
        index,
        plane.sizeInBytes, plane.rowStrideInBytes, rowNum
    );
    //
    return isEqual;
}


/**
 *
 */
status_t
queryStaticInfo(buffer_handle_t const bh, MyStaticInfo& info)
{
    struct Extra {
        int     format;
        int     usage;
        size_t  widthInPixels;
        size_t  heightInPixels;
        size_t  rowStrideInPixels;
        size_t  rowNum;
        size_t  allocSize;
    } extra;
    {
        int err = GRALLOC_EXTRA_OK;

#define MY_QUERY(_extra_get_, _field_) \
    extra._field_ = 0; \
    err |= ::gralloc_extra_query(bh, GRALLOC_EXTRA_GET_##_extra_get_, &extra._field_)
        //
        MY_QUERY(FORMAT         , format);
        MY_QUERY(USAGE          , usage);
        MY_QUERY(WIDTH          , widthInPixels);
        MY_QUERY(HEIGHT         , heightInPixels);
        MY_QUERY(STRIDE         , rowStrideInPixels);
        MY_QUERY(VERTICAL_STRIDE, rowNum);
        MY_QUERY(ALLOC_SIZE     , allocSize);
        //
#undef  MY_QUERY
        //
        if  ( GRALLOC_EXTRA_OK != err) {
            MY_LOGE("Given buffer_handle_t:%p, gralloc_extra_query returns %d", bh, err);
            return UNKNOWN_ERROR;
        }

    }
    //
    info.format         = extra.format;
    info.usage          = extra.usage;
    info.widthInPixels  = extra.widthInPixels;
    info.heightInPixels = extra.heightInPixels;
    info.allocSize      = extra.allocSize;
    //
    switch  ( info.format )
    {
    case HAL_PIXEL_FORMAT_Y8:
    case HAL_PIXEL_FORMAT_BLOB:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.sizeInBytes = info.allocSize;
                plane.rowStrideInBytes = (extra.rowStrideInPixels);     //1bpp
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        }break;
    //
    case HAL_PIXEL_FORMAT_YCbCr_422_I:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = (extra.rowStrideInPixels<<1);  //2bpp
                plane.sizeInBytes = info.allocSize;
                //
                checkPlaneSize(info, 0, (extra.rowNum));
                MY_LOGW_IF(plane.rowStrideInBytes%32!=0, "[YCbCr_422_I] rowStrideInBytes:%d%32!=0", plane.rowStrideInBytes);
                MY_LOGW_IF(plane.sizeInBytes<plane.rowStrideInBytes*16, "[YCbCr_422_I] sizeInBytes:%d<rowStrideInBytes:%d*16", plane.sizeInBytes, plane.rowStrideInBytes);
            }
        }break;
    //
    case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        {
            struct android_ycbcr ycbcr;
            if  ( OK != gQueryYCbCr(ycbcr, bh, info.widthInPixels, info.heightInPixels) ) {
                return UNKNOWN_ERROR;
            }
            //
            info.planes.resize(2);
            //
            {// Y
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = ycbcr.ystride;
                plane.sizeInBytes = ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
            {// CrCb
                MyStaticInfo::Plane& plane = info.planes.editItemAt(1);
                plane.rowStrideInBytes = ycbcr.cstride;
                plane.sizeInBytes = extra.allocSize - ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
                //
                checkPlaneSize(info, 1, (extra.rowNum>>1));
            }
        }break;
    //
    case HAL_PIXEL_FORMAT_YV12:
        {
            struct android_ycbcr ycbcr;
            if  ( OK != gQueryYCbCr(ycbcr, bh, info.widthInPixels, info.heightInPixels) ) {
                return UNKNOWN_ERROR;
            }
            //
            info.planes.resize(3);
            //
            {// Y
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = ycbcr.ystride;
                plane.sizeInBytes = ADDRESS_OFFSET(ycbcr.cr, ycbcr.y);
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
            {// Cr
                MyStaticInfo::Plane& plane = info.planes.editItemAt(1);
                plane.rowStrideInBytes = ycbcr.cstride;
                plane.sizeInBytes = ADDRESS_OFFSET(ycbcr.cb, ycbcr.cr);
                //
                checkPlaneSize(info, 1, (extra.rowNum>>1));
            }
            {// Cb
                MyStaticInfo::Plane& plane = info.planes.editItemAt(2);
                plane.rowStrideInBytes = ycbcr.cstride;
                plane.sizeInBytes = extra.allocSize - ADDRESS_OFFSET(ycbcr.cb, ycbcr.y);
                //
                checkPlaneSize(info, 2, (extra.rowNum>>1));
            }
        }break;
    //
    case HAL_PIXEL_FORMAT_RGBA_8888:
    case HAL_PIXEL_FORMAT_BGRA_8888:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = (extra.rowStrideInPixels<<2);//4bpp
                plane.sizeInBytes = extra.allocSize;
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        }break;
    case HAL_PIXEL_FORMAT_RAW16:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = (extra.rowStrideInPixels<<1);//2bpp
                plane.sizeInBytes = extra.allocSize;
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        }break;
    case HAL_PIXEL_FORMAT_RAW_OPAQUE:
        {
            info.planes.resize(1);
            //
            {
                MyStaticInfo::Plane& plane = info.planes.editItemAt(0);
                plane.rowStrideInBytes = (extra.rowStrideInPixels);//1bpp
                plane.sizeInBytes = extra.allocSize;
                //
                checkPlaneSize(info, 0, (extra.rowNum));
            }
        }break;
    //
    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
    case HAL_PIXEL_FORMAT_YCbCr_420_888:
        MY_LOGE(
            "format from gralloc_extra_query should not be: %#x(%s)",
            info.format,
            gQueryPixelFormatName(info.format).string()
        );
        return UNKNOWN_ERROR;
    default:
        MY_LOGE(
            "Not support format: %#x(%s)",
            info.format,
            gQueryPixelFormatName(info.format).string()
        );
        return NAME_NOT_FOUND;
    }
    //
    return OK;
}
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class GrallocHelperImp : public IGrallocHelper
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     *
     */
    struct  MyRequest : public GrallocRequest
    {
                                MyRequest(GrallocRequest const& rhs = GrallocRequest())
                                    : GrallocRequest(rhs)
                                {
                                }

        bool                    operator==(MyRequest const& rhs) const
                                {
                                    return  usage == rhs.usage
                                        &&  format== rhs.format
                                        &&  widthInPixels == rhs.widthInPixels
                                        &&  heightInPixels== rhs.heightInPixels
                                            ;
                                }

        bool                    operator<(MyRequest const& rhs) const
                                {
                                    /*
                                        true : if lhs < rhs
                                        false: if lhs >= rhs
                                     */
                                    if  ( format != rhs.format ) {
                                        return  format < rhs.format;
                                    }
                                    if  ( usage != rhs.usage ) {
                                        return  usage < rhs.usage;
                                    }
                                    if  ( widthInPixels != rhs.widthInPixels ) {
                                        return  widthInPixels < rhs.widthInPixels;
                                    }
                                    if  ( heightInPixels != rhs.heightInPixels ) {
                                        return  heightInPixels < rhs.heightInPixels;
                                    }
                                    //
                                    //  here: lhs >= rhs
                                    return  false;
                                }
    };

    typedef KeyedVector<MyRequest, MyStaticInfo> Map_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Data Members.

    mutable Mutex               mLock;
    mutable Map_t               mMap;

protected:  ////                Operations.
    static  status_t            determine(GrallocRequest const& src, MyRequest& dst);
    static  status_t            determine(MyRequest const& src, MyStaticInfo& dst);

    ssize_t                     addToMapLocked(
                                    Map_t::key_type const& k,
                                    Map_t::value_type const& v
                                ) const;

    void                        dump(
                                    Map_t::key_type const& k,
                                    Map_t::value_type const& v
                                ) const;

public:     ////                Operations.
                                GrallocHelperImp();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IGrallocHelper Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Operations.

    virtual int                 query(
                                    struct GrallocRequest const*    pRequest,
                                    struct GrallocStaticInfo*       pStaticInfo
                                ) const;

    virtual int                 query(
                                    buffer_handle_t                 bufHandle,
                                    struct GrallocStaticInfo*       pStaticInfo,
                                    struct GrallocDynamicInfo*      pDynamicInfo
                                ) const;

    virtual int                 setBufferStatus(
                                    buffer_handle_t                 bufHandle,
                                    int                             mask,
                                    int                             value
                                ) const;

    virtual int                 setDirtyCamera(
                                    buffer_handle_t                 bufHandle
                                ) const;

    virtual int                 setColorspace_JFIF(
                                    buffer_handle_t                 bufHandle
                                ) const;

    virtual String8             queryPixelFormatName(int format) const
                                {
                                    return  gQueryPixelFormatName(format);
                                }

    virtual String8             queryGrallocUsageName(int usage) const
                                {
                                    return  gQueryGrallocUsageName(usage);
                                }

    virtual void                dumpToLog() const;

};


static GrallocHelperImp     gSingleton;


};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
IGrallocHelper*
IGrallocHelper::
singleton()
{
    return &gSingleton;
}


/******************************************************************************
 *
 ******************************************************************************/
GrallocHelperImp::
GrallocHelperImp()
{
    char value[PROPERTY_VALUE_MAX];
    ::property_get("debug.mmutils.gralloc.loglevel", value, "0");
    setLogLevel(::atoi(value));
}


/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
setBufferStatus(
    buffer_handle_t                 bufHandle,
    int                             mask,
    int                             value
) const
{
    gralloc_extra_ion_sf_info_t info;
    gralloc_extra_query(bufHandle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &info);
    gralloc_extra_sf_set_status(&info, mask, value);
    return gralloc_extra_perform(bufHandle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &info);
}


/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
setDirtyCamera(
    buffer_handle_t                 bufHandle
) const
{
    return setBufferStatus(
        bufHandle,
        GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY,
        GRALLOC_EXTRA_BIT_TYPE_CAMERA | GRALLOC_EXTRA_BIT_DIRTY
    );
}


/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
setColorspace_JFIF(
    buffer_handle_t                 bufHandle
) const
{
    return setBufferStatus(
        bufHandle,
        GRALLOC_EXTRA_MASK_YUV_COLORSPACE,
        GRALLOC_EXTRA_BIT_YUV_BT601_FULL
    );
}


/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
query(
    struct GrallocRequest const*    pRequest,
    struct GrallocStaticInfo*       pStaticInfo
) const
{
    status_t status = OK;
    //
    if  ( ! pRequest ) {
        MY_LOGE("NULL pRequest");
        return  BAD_VALUE;
    }
    //
    if  ( ! pStaticInfo ) {
        MY_LOGE("NULL pStaticInfo");
        return  BAD_VALUE;
    }
    //
    MyRequest request;
    determine(*pRequest, request);
    //
    Mutex::Autolock _l(mLock);
    ssize_t const index = mMap.indexOfKey(request);
    //
    if  ( index < 0 )
    {
        MyStaticInfo staticInfo;
        status = determine(request, staticInfo);
        if  ( OK == status ) {
            addToMapLocked(request, staticInfo);
            *pStaticInfo = staticInfo;
        }
        else {
            return  status;
        }
    }
    else
    {
        *pStaticInfo = mMap.valueAt(index);
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
int
GrallocHelperImp::
query(
    buffer_handle_t                 bufHandle,
    struct GrallocStaticInfo*       pStaticInfo,
    struct GrallocDynamicInfo*      pDynamicInfo
) const
{
    status_t status = OK;
    //
    if  ( ! bufHandle ) {
        MY_LOGE("NULL buffer_handle_t");
        return  BAD_VALUE;
    }
    //
    //
    MyStaticInfo staticInfo;
    status = queryStaticInfo(bufHandle, staticInfo);
    if  ( OK != status ) {
        return  status;
    }
    //
    //  Add it to map if not existing.
    {
        MyRequest request;
        request.usage = 0;
        request.format        = staticInfo.format;
        request.widthInPixels = staticInfo.widthInPixels;
        request.heightInPixels= staticInfo.heightInPixels;
        //
        Mutex::Autolock _l(mLock);
        ssize_t const index = mMap.indexOfKey(request);
        if  ( index < 0 )
        {
            addToMapLocked(request, staticInfo);
        }
    }
    //
    if  ( NULL != pStaticInfo ) {
        *pStaticInfo = staticInfo;
    }
    //
    if  ( NULL != pDynamicInfo ) {
        Vector<int>& ionFds = pDynamicInfo->ionFds;
        //
        int ionFd = -1;
        int err = ::gralloc_extra_query(bufHandle, GRALLOC_EXTRA_GET_ION_FD, &ionFd);
        if  ( GRALLOC_EXTRA_OK == err && 0 <= ionFd ) {
            ionFds.push_back(ionFd);
        }
        else {
            MY_LOGE("[buffer_handle_t:%p] err:%d ionFd:%d", bufHandle, err, ionFd);
            return  UNKNOWN_ERROR;
        }
    }
    //
    return status;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
GrallocHelperImp::
determine(GrallocRequest const& src, MyRequest& dst)
{
    dst = src;
    //
    switch  (src.format)
    {
    case HAL_PIXEL_FORMAT_IMPLEMENTATION_DEFINED:
    case HAL_PIXEL_FORMAT_YCbCr_420_888:
        dst.usage = src.usage;
        break;
    default:
        dst.usage = 0;
        break;
    }
    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
GrallocHelperImp::
determine(MyRequest const& src, MyStaticInfo& dst)
{
    nsecs_t const startTime = ::systemTime();
    sp<GraphicBuffer> pGB =
    new GraphicBuffer(
        src.widthInPixels,
        src.heightInPixels,
        src.format,
        src.usage | GRALLOC_USAGE_NULL_BUFFER
    );
    if  ( pGB == 0 || pGB->getNativeBuffer() == 0 || ! pGB->getNativeBuffer()->handle ) {
        MY_LOGE(
            "Bad GraphicBuffer:%p getNativeBuffer:%p handle:%p %dx%d format:%#x(%s) usage:%#x(%s)",
            pGB.get(), pGB->getNativeBuffer(), pGB->getNativeBuffer()->handle,
            src.widthInPixels, src.heightInPixels,
            src.format, gQueryPixelFormatName(src.format).string(),
            src.usage, gQueryGrallocUsageName(src.usage).string()
        );
        return  NO_MEMORY;
    }
    nsecs_t const grallocTime = ::systemTime() - startTime;
    //
    dst.grallocTime = grallocTime;
    return  queryStaticInfo(pGB->getNativeBuffer()->handle, dst);
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
GrallocHelperImp::
addToMapLocked(
    Map_t::key_type const& k,
    Map_t::value_type const& v
) const
{
    dump(k, v);
    return mMap.add(k, v);
}


/******************************************************************************
 *
 ******************************************************************************/
void
GrallocHelperImp::
dumpToLog() const
{
    MY_LOGD("+");
    Mutex::Autolock _l(mLock);
    for (size_t i = 0; i < mMap.size(); i++) {
        dump(mMap.keyAt(i), mMap.valueAt(i));
    }
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
void
GrallocHelperImp::
dump(
    Map_t::key_type const& k,
    Map_t::value_type const& v
) const
{
    MY_LOGD("*****************************************************************************");
    MY_LOGD("gralloc time: %lld us", ::nanoseconds_to_microseconds(v.grallocTime));
    MY_LOGD(
        "%dx%d usage:0x%08x(%s) format:0x%08x(%s) -->",
        k.widthInPixels, k.heightInPixels,
        k.usage, queryGrallocUsageName(k.usage).string(),
        k.format, queryPixelFormatName(k.format).string()
    );
    MY_LOGD(
        "%dx%d usage:0x%08x(%s) format:0x%08x(%s)",
        v.widthInPixels, v.heightInPixels,
        v.usage, queryGrallocUsageName(v.usage).string(),
        v.format, queryPixelFormatName(v.format).string()
    );
    for (size_t i = 0; i < v.planes.size(); i++) {
        MY_LOGD(
            "  [%zu] sizeInBytes:%zu rowStrideInBytes:%zu",
            i,
            v.planes[i].sizeInBytes,
            v.planes[i].rowStrideInBytes
        );
    }
}

