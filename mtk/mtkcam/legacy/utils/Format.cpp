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

#define LOG_TAG "MtkCam/Utils"
//
#include "MyUtils.h"
#include <camera/MtkCameraParameters.h>
//
using namespace android;


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
namespace NSCam {
namespace Utils {
namespace Format {


/******************************************************************************
 *  String-Enum Format Map Manager
 ******************************************************************************/
struct  StrEnumMapMgr
{
public:     ////            Data Members.
    typedef DefaultKeyedVector<String8, EImageFormat>   MapType;
    MapType                 mvMapper;
    mutable Mutex           mLock;

public:     ////            Instantiation.
                            StrEnumMapMgr()
                                : mvMapper(eImgFmt_UNKNOWN)
                                , mLock()
                            {
                                mvMapper.clear();
                                Mutex::Autolock _l(mLock);
                                //
#define DO_MAP(_format_, _eImageFormat) \
            do { \
                mvMapper.add(String8(MtkCameraParameters::PIXEL_FORMAT_##_format_), eImgFmt_##_eImageFormat); \
            } while (0)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            DO_MAP(YUV420SP         , NV21);
                            DO_MAP(YUV420P          , YV12);
                            DO_MAP(YV12_GPU         , YV12);
                            DO_MAP(YUV420I          , I420);
                            DO_MAP(YUV422SP         , NV16);
                            DO_MAP(YUV422I          , YUY2);
                            DO_MAP(YUV422I_YVYU     , YVYU);
                            DO_MAP(YUV422I_UYVY     , UYVY);
                            DO_MAP(YUV422I_VYUY     , VYUY);
                            DO_MAP(RGB565           , RGB565);
                            DO_MAP(RGBA8888         , RGBA8888);
                            DO_MAP(BAYER8           , BAYER8);
                            DO_MAP(BAYER10          , BAYER10);
                            DO_MAP(JPEG             , JPEG);
//------------------------------------------------------------------------------
#undef  DO_MAP
                            }
};
static  StrEnumMapMgr       gStrEnumMap;


/*****************************************************************************
 * @brief Query the image format constant.
 *
 * @details Given a MtkCameraParameters::PIXEL_FORMAT_xxx, return its
 * corresponding image format constant.
 *
 * @note
 *
 * @param[in] szPixelFormat: A null-terminated string for pixel format (i.e.
 * MtkCameraParameters::PIXEL_FORMAT_xxx)
 *
 * @return its corresponding image format.
 *
 ******************************************************************************/
int
queryImageFormat(
    char const* szPixelFormat
)
{
    MY_LOGF_IF(NULL==szPixelFormat, "NULL==szPixelFormat");
    String8 const s8Format(szPixelFormat);
    int format = gStrEnumMap.mvMapper.valueFor(s8Format);
    MY_LOGW_IF(
        eImgFmt_UNKNOWN==format,
        "Unsupported MtkCameraParameters::PIXEL_FORMAT_xxx (%s)", szPixelFormat
    );
    return  format;
}


/******************************************************************************
 *  Image Format Interface
 ******************************************************************************/
class IImgFmt : public RefBase
{
public:     ////            Instantiation.
    virtual                 ~IImgFmt() {}

public:     ////            Attributes.
    virtual char const*     getName() const                                 = 0;
    virtual size_t          getPlaneCount() const                           = 0;
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const   = 0;
    virtual size_t          getPlaneHeightInPixels(size_t planeIndex, size_t heightInPixels)const   = 0;
    virtual int             getPlaneBitsPerPixel(size_t planeIndex) const   = 0;
    virtual int             getImageBitsPerPixel() const                    = 0;
};


/******************************************************************************
 *  Image Format Base
 ******************************************************************************/
class CBaseImgFmt : public IImgFmt
{
protected:  ////            Data Members.
    String8                 ms8About;           //  "About" this class
    String8                 ms8Name;
    EImageFormat            meImageFormat;
    size_t                  mPlaneCount;
    int                     mImageBitsPerPixel;
    int                     mPlaneBitsPerPixel[3];

public:     ////            Instantiation.
                            CBaseImgFmt(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       imageBitsPerPixel,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1 = 0,
                                int const       planeBitsPerPixel2 = 0
                            )
                                : ms8About(String8(szAbout))
                                , ms8Name(String8(szName))
                                , meImageFormat(eImageFormat)
                                , mPlaneCount(planeCount)
                                , mImageBitsPerPixel(imageBitsPerPixel)
                            {
                                mPlaneBitsPerPixel[0] = planeBitsPerPixel0;
                                mPlaneBitsPerPixel[1] = planeBitsPerPixel1;
                                mPlaneBitsPerPixel[2] = planeBitsPerPixel2;
                            }

public:     ////            Attributes.
    virtual char const*     getName() const                                     { return ms8Name.string(); }
    virtual size_t          getPlaneCount() const                               { return mPlaneCount; }
    virtual int             getPlaneBitsPerPixel(size_t planeIndex) const       { return mPlaneBitsPerPixel[planeIndex]; }
    virtual int             getImageBitsPerPixel() const                        { return mImageBitsPerPixel; }
};


/******************************************************************************
 *  Image Format Class (1 Plane)
 ******************************************************************************/
class CBaseImgFmt_1Plane : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_1Plane(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                int const       imageBitsPerPixel
                            )
                                : CBaseImgFmt(szAbout, szName, eImageFormat, 1, imageBitsPerPixel, imageBitsPerPixel)
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const   { return widthInPixels; }
    virtual size_t          getPlaneHeightInPixels(size_t planeIndex, size_t heightInPixels)const   { return heightInPixels; }
};


#define DECLARE_FORMAT_1PLANE(_eImageFormat, _bitsPerPixel, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_1Plane \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_1Plane(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    _bitsPerPixel \
                ) \
        {} \
    }

//
//DECLARE_FORMAT_xxxx(IMPLEMENTATION_DEFINED,  0, 0, "Implementation Defined");
//DECLARE_FORMAT_xxxx(UNKNOWN               ,  0, 0, "Unknown");
//
DECLARE_FORMAT_1PLANE(YUY2                  , 16, "YUY2, 1 plane (YUYV), 422i");
DECLARE_FORMAT_1PLANE(YVYU                  , 16, "YVYU, 1 plane (YVYU), 422");
DECLARE_FORMAT_1PLANE(UYVY                  , 16, "UYVY, 1 plane (UYVY), 422");
DECLARE_FORMAT_1PLANE(VYUY                  , 16, "VYUY, 1 plane (VYUY), 422");

DECLARE_FORMAT_1PLANE(Y16                   , 16, "16-bit Y plane, 1 plane");
DECLARE_FORMAT_1PLANE(Y8                    ,  8, " 8-bit Y plane, 1 plane");

DECLARE_FORMAT_1PLANE(RGB565                , 16, "RGB565, 1 plane");
DECLARE_FORMAT_1PLANE(RGB888                , 24, "RGB888, 1 plane, LSB:R, MSB:B");
DECLARE_FORMAT_1PLANE(RGBA8888              , 32, "RGBA8888, 1 plane, LSB:R, MSB:A");
DECLARE_FORMAT_1PLANE(RGBX8888              , 32, "RGBX8888, 1 plane, LSB:R, MSB:X");
DECLARE_FORMAT_1PLANE(BGRA8888              , 32, "BGRA8888, 1 plane, LSB:B, MSB:A");
DECLARE_FORMAT_1PLANE(ARGB8888              , 32, "ARGB8888, 1 plane, A:LSB, B:MSB");

DECLARE_FORMAT_1PLANE(BLOB                  ,  8, "BLOB (width=size, height=1)");

DECLARE_FORMAT_1PLANE(JPEG                  ,  8, "JPEG");


DECLARE_FORMAT_1PLANE(BAYER8                ,  8, "Bayer8");
DECLARE_FORMAT_1PLANE(BAYER10               , 10, "Bayer10");
DECLARE_FORMAT_1PLANE(BAYER12               , 12, "Bayer12");
//DECLARE_FORMAT_1PLANE(RAW_SENSOR            , 16, "RAW SENSOR");
DECLARE_FORMAT_1PLANE(RAW16                    , 16, "RAW16");
DECLARE_FORMAT_1PLANE(FG_BAYER8             ,  8*3/2, "FG_Bayer8");
DECLARE_FORMAT_1PLANE(FG_BAYER10            , 10*3/2, "FG_Bayer10");
DECLARE_FORMAT_1PLANE(FG_BAYER12            , 12*3/2, "FG_Bayer12");
DECLARE_FORMAT_1PLANE(FG_BAYER14            , 14*3/2, "FG_Bayer14");



/******************************************************************************
 *  Image Format Class (YUV420 2/3 Plane, 12-bit)
 ******************************************************************************/
class CBaseImgFmt_YUV420_2_3_Plane_12Bits : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_YUV420_2_3_Plane_12Bits(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1,
                                int const       planeBitsPerPixel2 = 0
                            )
                                :   CBaseImgFmt(
                                        szAbout, szName, eImageFormat, planeCount,
                                        12, planeBitsPerPixel0, planeBitsPerPixel1, planeBitsPerPixel2
                                    )
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const
                            {
                                return  (0 == planeIndex)
                                            ? (widthInPixels)
                                            : (widthInPixels >> 1)
                                            ;
                            }

    virtual size_t          getPlaneHeightInPixels(size_t planeIndex, size_t heightInPixels)const
                            {
                                return  (0 == planeIndex)
                                            ? (heightInPixels)
                                            : (heightInPixels >> 1)
                                            ;
                            }
};

#define DECLARE_FORMAT_YUV420_2PLANE_12BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV420_2_3_Plane_12Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV420_2_3_Plane_12Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    2, _planeBitsPerPixel0, _planeBitsPerPixel1 \
                ) \
        {} \
    }

#define DECLARE_FORMAT_YUV420_3PLANE_12BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV420_2_3_Plane_12Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV420_2_3_Plane_12Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    3, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2 \
                ) \
        {} \
    }

DECLARE_FORMAT_YUV420_2PLANE_12BITS(NV21    ,  8, 16,       "NV21, 2 plane (Y)(VU), 420sp");
DECLARE_FORMAT_YUV420_2PLANE_12BITS(NV12    ,  8, 16,       "NV12, 2 plane (Y)(UV), 420");
DECLARE_FORMAT_YUV420_2PLANE_12BITS(NV21_BLK,  8, 16,       "NV21 block, 2 plane (Y)(VU), 420");
DECLARE_FORMAT_YUV420_2PLANE_12BITS(NV12_BLK,  8, 16,       "NV12 block, 2 plane (Y)(UV), 420");
//
DECLARE_FORMAT_YUV420_3PLANE_12BITS(YV12    ,  8,  8,  8,   "YV12, 3 plane (Y)(V)(U), 420p");
DECLARE_FORMAT_YUV420_3PLANE_12BITS(I420    ,  8,  8,  8,   "I420, 3 plane (Y)(U)(V)");


/******************************************************************************
 *  Image Format Class (YUV422 2/3 Plane, 16-bit)
 ******************************************************************************/
class CBaseImgFmt_YUV422_2_3_Plane_16Bits : public CBaseImgFmt
{
public:     ////            Instantiation.
                            CBaseImgFmt_YUV422_2_3_Plane_16Bits(
                                char const*     szAbout,
                                char const*     szName,
                                EImageFormat    eImageFormat,
                                size_t          planeCount,
                                int const       planeBitsPerPixel0,
                                int const       planeBitsPerPixel1,
                                int const       planeBitsPerPixel2 = 0
                            )
                                :   CBaseImgFmt(
                                        szAbout, szName, eImageFormat, planeCount,
                                        16, planeBitsPerPixel0, planeBitsPerPixel1, planeBitsPerPixel2
                                    )
                            {
                            }
public:     ////            Attributes.
    virtual size_t          getPlaneWidthInPixels(size_t planeIndex, size_t widthInPixels)  const
                            {
                                return  (0 == planeIndex)
                                            ? (widthInPixels)
                                            : (widthInPixels >> 1)
                                            ;
                            }

    virtual size_t          getPlaneHeightInPixels(size_t planeIndex, size_t heightInPixels)const
                            {
                                return  (heightInPixels);
                            }
};

#define DECLARE_FORMAT_YUV422_2PLANE_16BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV422_2_3_Plane_16Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV422_2_3_Plane_16Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    2, _planeBitsPerPixel0, _planeBitsPerPixel1 \
                ) \
        {} \
    }

#define DECLARE_FORMAT_YUV422_3PLANE_16BITS(_eImageFormat, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2, _about) \
    struct CImgFmt##_eImageFormat : public CBaseImgFmt_YUV422_2_3_Plane_16Bits \
    { \
        CImgFmt##_eImageFormat() \
            :   CBaseImgFmt_YUV422_2_3_Plane_16Bits(\
                    _about, #_eImageFormat, eImgFmt_##_eImageFormat, \
                    3, _planeBitsPerPixel0, _planeBitsPerPixel1, _planeBitsPerPixel2 \
                ) \
        {} \
    }

DECLARE_FORMAT_YUV422_2PLANE_16BITS(NV61    ,  8, 16,       "NV61, 2 plane (Y)(VU), 422");
DECLARE_FORMAT_YUV422_2PLANE_16BITS(NV16    ,  8, 16,       "NV16, 2 plane (Y)(UV), 422sp");
//
DECLARE_FORMAT_YUV422_3PLANE_16BITS(YV16    ,  8,  8,  8,   "YV16, 3 plane (Y)(V)(U), 422");
//
DECLARE_FORMAT_YUV422_3PLANE_16BITS(I422    ,  8,  8,  8,   "I422, 3 plane (Y)(V)(U), 422");


/******************************************************************************
 *  Image Format Map Manager
 ******************************************************************************/
struct MapMgr
{
public:     ////            Data Members.
    typedef DefaultKeyedVector<int, sp<IImgFmt> >  MapType;
    MapType                 mvMapper;
public:     ////            Instantiation.
                            MapMgr()
                                : mvMapper(NULL)
                            {
                                mvMapper.clear();
                                //
#define DO_MAP(_eImageFormat)    \
            do { \
                mvMapper.add(eImgFmt_##_eImageFormat, new CImgFmt##_eImageFormat());\
            } while (0)
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//DO_MAP(IMPLEMENTATION_DEFINED);
//DO_MAP(UNKNOWN);
//
DO_MAP(YV12);
DO_MAP(I420);
DO_MAP(NV21);
DO_MAP(NV12);
DO_MAP(NV12_BLK);
DO_MAP(NV21_BLK);
//
DO_MAP(YV16);
DO_MAP(NV61);
DO_MAP(NV16);
DO_MAP(I422);
//
DO_MAP(YUY2);
DO_MAP(YVYU);
DO_MAP(UYVY);
DO_MAP(VYUY);
//
DO_MAP(Y16);
DO_MAP(Y8);
//
DO_MAP(RGB565);
DO_MAP(RGB888);
DO_MAP(RGBA8888);
DO_MAP(RGBX8888);
DO_MAP(BGRA8888);
DO_MAP(ARGB8888);
//
DO_MAP(BLOB);
//
DO_MAP(JPEG);
//
DO_MAP(BAYER8);
DO_MAP(BAYER10);
DO_MAP(BAYER12);
//DO_MAP(RAW_SENSOR);
DO_MAP(RAW16);
//
DO_MAP(FG_BAYER8);
DO_MAP(FG_BAYER10);
DO_MAP(FG_BAYER12);
DO_MAP(FG_BAYER14);


//------------------------------------------------------------------------------
#undef  DO_MAP
                            }
};

static MapMgr mapMgrInstance;

MapMgr&
getMapMgr()
{
    return mapMgrInstance;
}


MapMgr::MapType
getMapper()
{
    return  getMapMgr().mvMapper;
}

bool
checkValidFormat(
    int const imageFormat
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        return true;
    }
    else
    {
        dumpMapInformation();
        MY_LOGW("Unsupported Image Format: %#x", imageFormat);
        return false;
    }
}

/*****************************************************************************
 * @brief Query the plane count.
 *
 * @details Given a format of type EImageFormat, return its corresponding
 * plane count.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 *
 * @return its corresponding plane count.
 *
 ******************************************************************************/
size_t
queryPlaneCount(
    int const imageFormat
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        size_t const PlaneCount = p->getPlaneCount();
        MY_LOGV("[%s] plane count %d", p->getName(), PlaneCount);
        return  PlaneCount;
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query the width in pixels of a specific plane.
 *
 * @details Given a format of type EImageFormat, a plane index, and the width in
 * in pixels of the 0-th plane, return the width in pixels of the given plane.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 * @param[in] planeIndex: a specific plane index.
 * @param[in] widthInPixels: the width in pixels of the 0-th plane.
 *
 * @return the width in pixels of the given plane.
 *
 ******************************************************************************/
size_t
queryPlaneWidthInPixels(
    int const imageFormat,
    size_t planeIndex,
    size_t widthInPixels
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        size_t const pixels = p->getPlaneWidthInPixels(planeIndex, widthInPixels);
        MY_LOGV("[%s] Width in pixels %d @ %d-th plane", p->getName(), pixels, planeIndex);
        return  pixels;
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query the height in pixels of a specific plane.
 *
 * @details Given a format of type EImageFormat, a plane index, and the height
 * in pixels of the 0-th plane, return the height in pixels of the given plane.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 * @param[in] planeIndex: a specific plane index.
 * @param[in] heightInPixels: the height in pixels of the 0-th plane.
 *
 * @return the height in pixels of the given plane.
 *
 ******************************************************************************/
size_t
queryPlaneHeightInPixels(
    int const imageFormat,
    size_t planeIndex,
    size_t heightInPixels
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        size_t const pixels = p->getPlaneHeightInPixels(planeIndex, heightInPixels);
        MY_LOGV("[%s] Height in pixels %d @ %d-th plane", p->getName(), pixels, planeIndex);
        return  pixels;
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query the bits per pixel of a specific plane.
 *
 * @details Given a format of type EImageFormat and a plane index, return
 * the bits per pixel of the given plane.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 * @param[in] planeIndex: a specific plane index.
 *
 * @return the bits per pixel of the given plane.
 *
 ******************************************************************************/
int
queryPlaneBitsPerPixel(
    int const imageFormat,
    size_t planeIndex
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        int const bitsPerPixel = p->getPlaneBitsPerPixel(planeIndex);
        MY_LOGV("[%s] bits per pixel %d @ %d-th plane", p->getName(), bitsPerPixel, planeIndex);
        return  bitsPerPixel;
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  0;
}


/*****************************************************************************
 * @brief Query the bits per pixel of a specific plane.
 *
 * @details Given a format of type EImageFormat and a plane index, return
 * the bits per pixel of the given plane.
 *
 * @note
 *
 * @param[in] imageFormat: A format of type EImageFormat.
 * @param[in] planeIndex: a specific plane index.
 *
 * @return the bits per pixel of the given plane.
 *
 ******************************************************************************/
int
queryImageBitsPerPixel(
    int const imageFormat
)
{
    sp<IImgFmt> p = getMapper().valueFor(imageFormat);
    if  ( p != 0 )
    {
        int const bitsPerPixel = p->getImageBitsPerPixel();
        MY_LOGV("[%s] bits per pixel %d", p->getName(), bitsPerPixel);
        return  bitsPerPixel;
    }
    //
    dumpMapInformation();
    MY_LOGW("Unsupported Image Format: %#x", imageFormat);
    return  0;
}

/*****************************************************************************
 * @Dump the information of Image map.
 ******************************************************************************/
void
dumpMapInformation()
{
    size_t mapSize = getMapper().size();
    if ( mapSize == 0 )
    {
        MY_LOGW("map size is 0");
    }
    else
    {
        int key;
        sp<IImgFmt> keyValue;
        MY_LOGW("map size is %d", mapSize);
        for( int i = 0; i<mapSize; i++)
        {
            key = getMapper().keyAt(i);
            keyValue = getMapper().valueAt(i);
            MY_LOGW("Index (%d): Key = %#x, value = %s", i, key, keyValue->getName());
        }
    }
}


};  // namespace Format
};  // namespace Utils
};  // namespace NSCam

