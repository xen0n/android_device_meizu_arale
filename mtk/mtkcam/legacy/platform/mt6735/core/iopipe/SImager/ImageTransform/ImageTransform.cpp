/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/ImageTranform"
//
#include "MyUtils.h"
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define FUNCTION_LOG_START      MY_LOGD("+");
#define FUNCTION_LOG_END        MY_LOGD("-");

//
#include <DpIspStream.h>
//
#include "./inc/ImageTransform.h"

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}
#define CHECK_DP_RET(ret, x)                        \
    do{                                             \
        if( ret && ((x) < 0) )                      \
        {                                           \
            MY_LOGE("%s failed", #x); ret = MFALSE; \
        }                                           \
    }while(0)

using namespace android;
using namespace NSCam::NSIoPipe::NSSImager;
/*******************************************************************************
*
********************************************************************************/

////////////////////////////////////////////////////////////////////////////////
#define DP_PORT_SRC   4
#define DP_PORT_DST0  0
#define DP_PORT_DST1  1

/*******************************************************************************
*
********************************************************************************/
ImageTransform::
ImageTransform(
)
    : mi4ErrorCode(0)
    , mpStream(NULL)
{
}


/*******************************************************************************
*
********************************************************************************/
ImageTransform::
~ImageTransform(
)
{
    if( mpStream != NULL )
    {
         delete mpStream;
         mpStream = NULL;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
execute(
    IImageBuffer const * pSrcBuf,
    IImageBuffer const * pDstBuf_0,
    IImageBuffer const * pDstBuf_1,
    MRect const rROI,
    MUINT32 const u4Transform_0,
    MUINT32 const u4TimeOutInMs
)
{
    MY_LOGD("src %p, dst %p/%p, (%d, %d, %dx%d), trans %d +",
            pSrcBuf, pDstBuf_0, pDstBuf_1,
            rROI.p.x, rROI.p.y, rROI.s.w, rROI.s.h,
            u4Transform_0);
    MBOOL ret = MTRUE;
    DP_STATUS_ENUM dp_ret;

    if( !pSrcBuf || (!pDstBuf_0 && !pDstBuf_1) )
    {
        MY_LOGE("not config enough port, src(0x%x), dst0(0x%x), dst1(0x%x)",
                pSrcBuf, pDstBuf_0, pDstBuf_1);
        ret = MFALSE;
    }

    if (!lock(u4TimeOutInMs))
    {
        MY_LOGE("[execute] lock fail ");
        return MFALSE;
    }

    // (1). Create Instance
    if( mpStream == NULL )
    {
        mpStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
        CHECK_OBJECT( mpStream );
    }

    // (2). config/enque src
    if( pSrcBuf )
    {
        ret = ret && configPort(DP_PORT_SRC, pSrcBuf);
        CHECK_DP_RET( ret, mpStream->setSrcCrop( rROI.p.x, 0,
                                                 rROI.p.y, 0,
                                                 rROI.s.w, rROI.s.h
                                                 ) );
        ret = ret && enqueBuffer(DP_PORT_SRC, pSrcBuf);
    }

    // (3). config/enque dst
    if( pDstBuf_0 )
    {
        MUINT32 u4Rotation = 0;
        MUINT32 u4Flip = 0;

        ret = ret && configPort(DP_PORT_DST0, pDstBuf_0)
            && convertTransform(u4Transform_0, u4Rotation, u4Flip);

        CHECK_DP_RET( ret, mpStream->setRotation(DP_PORT_DST0, u4Rotation) );
        CHECK_DP_RET( ret, mpStream->setFlipStatus(DP_PORT_DST0, u4Flip) );

        ret = ret && enqueBuffer(DP_PORT_DST0, pDstBuf_0);
    }

    if( pDstBuf_1 )
    {
        ret = ret
            && configPort(DP_PORT_DST1, pDstBuf_1)
            && enqueBuffer(DP_PORT_DST1, pDstBuf_1);
    }

    // (4). start
    //profile.print();
    CHECK_DP_RET( ret, mpStream->startStream() );

    // (5). deque
    CHECK_DP_RET( ret, mpStream->dequeueSrcBuffer() );

    if( pDstBuf_0 ) {
        ret = ret && dequeDstBuffer(DP_PORT_DST0, pDstBuf_0 );
    }

    if( pDstBuf_1 ) {
        ret = ret && dequeDstBuffer(DP_PORT_DST1, pDstBuf_1 );
    }

    CHECK_DP_RET( ret, mpStream->dequeueFrameEnd() );


    // (6). stop
    CHECK_DP_RET( ret, mpStream->stopStream() );

    if( !ret && mpStream != NULL )
    {
        delete mpStream;
        mpStream = NULL;
    }
    //
    MY_LOGD("-");
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
lock(MUINT32 const u4TimeOutInMs)
{
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
unlock()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
convertTransform(
    MUINT32 const u4Transform,
    MUINT32 & u4Rotation,
    MUINT32 & u4Flip
)
{
    MY_LOGV("u4Transform:0x%x", u4Transform);
    switch (u4Transform)
    {
#define TransCase( trans, rot, flip ) \
        case (trans):                 \
            u4Rotation = (rot);       \
            u4Flip = (flip);          \
            break;
        TransCase(0                  , 0   , 0)
        TransCase(eTransform_FLIP_H  , 0   , 1)
        TransCase(eTransform_FLIP_V  , 180 , 1)
        TransCase(eTransform_ROT_90  , 90  , 0)
        TransCase(eTransform_ROT_180 , 180 , 0)
        TransCase(eTransform_FLIP_H|eTransform_ROT_90 , 270 , 1)
        TransCase(eTransform_FLIP_V|eTransform_ROT_90 , 90  , 1)
        TransCase(eTransform_ROT_270 , 270 , 0)
        default:
            MY_LOGE("not supported transform(0x%x)", u4Transform);
            u4Rotation = 0;
            u4Flip = 0;
            return MFALSE;
            break;
#undef TransCase
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
mapDpFormat(
    NSCam::EImageFormat fmt,
    DpColorFormat* dp_fmt
)
{
    switch( fmt )
    {
#define FMT_SUPPORT( fmt, dpfmt ) \
        case fmt: \
            *(dp_fmt) = dpfmt; \
            break;
        FMT_SUPPORT(eImgFmt_YUY2   , DP_COLOR_YUYV)
        FMT_SUPPORT(eImgFmt_UYVY   , DP_COLOR_UYVY)
        FMT_SUPPORT(eImgFmt_YVYU   , DP_COLOR_YVYU)
        FMT_SUPPORT(eImgFmt_VYUY   , DP_COLOR_VYUY)
        FMT_SUPPORT(eImgFmt_NV16   , DP_COLOR_NV16)
        FMT_SUPPORT(eImgFmt_NV61   , DP_COLOR_NV61)
        FMT_SUPPORT(eImgFmt_NV21   , DP_COLOR_NV21)
        FMT_SUPPORT(eImgFmt_NV12   , DP_COLOR_NV12)
        FMT_SUPPORT(eImgFmt_YV16   , DP_COLOR_YV16)
        FMT_SUPPORT(eImgFmt_I422   , DP_COLOR_I422)
        FMT_SUPPORT(eImgFmt_YV12   , DP_COLOR_YV12)
        FMT_SUPPORT(eImgFmt_I420   , DP_COLOR_I420)
        FMT_SUPPORT(eImgFmt_Y800   , DP_COLOR_GREY)
        FMT_SUPPORT(eImgFmt_RGB565 , DP_COLOR_RGB565)
        FMT_SUPPORT(eImgFmt_RGB888 , DP_COLOR_RGB888)
        FMT_SUPPORT(eImgFmt_ARGB888, DP_COLOR_ARGB8888)
        FMT_SUPPORT(eImgFmt_RGBA8888, DP_COLOR_RGBA8888)
        // not supported
        default:
            MY_LOGE(" fmt(0x%x) not support in DP", fmt);
            return MFALSE;
            break;
#undef FMT_SUPPORT
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
configPort(
    MUINT32 const port,
    IImageBuffer const *pImageBuffer
)
{
    MBOOL ret = MTRUE;
    DpColorFormat fmt;

    if( !mapDpFormat((NSCam::EImageFormat)pImageBuffer->getImgFormat(), &fmt) )
    {
        return MFALSE;
    }

    if( port == DP_PORT_SRC )
    {
        MUINT32 planenum = pImageBuffer->getPlaneCount();
        CHECK_DP_RET( ret, mpStream->setSrcConfig(
                                        pImageBuffer->getImgSize().w,
                                        pImageBuffer->getImgSize().h,
                                        pImageBuffer->getBufStridesInBytes(0),
                                        planenum > 1 ? pImageBuffer->getBufStridesInBytes(1) : 0,
                                        fmt,
                                        DP_PROFILE_FULL_BT601
                                        ) );
        MY_LOGD("Stride[0](%d)",
                pImageBuffer->getBufStridesInBytes(0));
    }
    else
    {
        MUINT32 planenum = pImageBuffer->getPlaneCount();
        CHECK_DP_RET( ret, mpStream->setDstConfig(
                                        port,
                                        pImageBuffer->getImgSize().w,
                                        pImageBuffer->getImgSize().h,
                                        pImageBuffer->getBufStridesInBytes(0),
                                        planenum > 1 ? pImageBuffer->getBufStridesInBytes(1) : 0,
                                        fmt,
                                        DP_PROFILE_FULL_BT601
                                        ) );
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
enqueBuffer(
    MUINT32 const port,
    IImageBuffer const *pImageBuffer
)
{
    MBOOL ret = MTRUE;
    uint32_t va[3];
    uint32_t pa[3];
    uint32_t size[3];

#define checkAddr( addr, i )                       \
    do{                                            \
        if( addr == 0 )                            \
        {                                          \
            MY_LOGE( "%s == 0, p(%d)", #addr, i ); \
            return MFALSE;                         \
        }                                          \
    }while(0)

    for( MUINT i = 0 ; i < pImageBuffer->getPlaneCount() ; i++ )
    {
        va[i]   = (MUINT32)pImageBuffer->getBufVA(i);
        pa[i]   = pImageBuffer->getBufPA(i);
        size[i] = pImageBuffer->getBufSizeInBytes(i);

//        checkAddr(va[i], i);
//        checkAddr(pa[i], i);
    }

    if( port == DP_PORT_SRC )
    {
        CHECK_DP_RET( ret, mpStream->queueSrcBuffer( (void**)va,
                                                     pa,
                                                     size,
                                                     pImageBuffer->getPlaneCount()
                                                     ) );
    }
    else
    {
        CHECK_DP_RET( ret, mpStream->queueDstBuffer( port,
                                                     (void**)va,
                                                     pa,
                                                     size,
                                                     pImageBuffer->getPlaneCount()
                                                     ) );
    }

#undef checkAddr
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImageTransform::
dequeDstBuffer(
    MUINT32 const port,
    IImageBuffer const *pImageBuffer
)
{
    MBOOL ret = MTRUE;

    uint32_t va[3];
    CHECK_DP_RET( ret, mpStream->dequeueDstBuffer( port, (void**)va, true ) );

    for( MUINT i = 0 ; i < pImageBuffer->getPlaneCount() ; i++ )
    {
        if( va[i] != (MUINT32)pImageBuffer->getBufVA(i) )
        {
            MY_LOGE("deque wrong buffer va(0x%x) != 0x%x",
                    va[i], pImageBuffer->getBufVA(i) );
            ret = MFALSE;
            break;
        }
    }

    return ret;
}
