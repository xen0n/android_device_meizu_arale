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
#define LOG_TAG "MtkCam/JpegCodec"
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
#include <cutils/properties.h>
#include <stdlib.h>
//
#include <mtkcam/utils/common.h>
#include <mtkcam/iopipe/SImager/ISImagerDataTypes.h>
#include <mtkcam/iopipe/SImager/IImageTransform.h>
//
#if 1==MTKCAM_USE_LEGACY_JPEGHW
// jpeg encoder
#include <enc/jpeg_hal.h>
#define JPEG_HAL_SUPPORT
#endif
//
// image transform
#include <mtkcam/utils/Format.h>
using namespace NSCam::Utils::Format;
//
#include "./inc/JpegCodec.h"

#define MEDIA_PATH "/sdcard/"


#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}
/*******************************************************************************
*
********************************************************************************/
using namespace android;
using namespace NSCam;
using namespace NSCam::NSIoPipe::NSSImager;

////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
JpegCodec::
JpegCodec(
)
    : mi4ErrorCode(0)
{
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.camera.dump", value, "0");
    mu4DumpFlag = ::atoi(value);
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegCodec::
encode(
    IImageBuffer const *pSrcBufInfo,
    IImageBuffer const *pDstBufInfo,
    MRect const rROI,
    MUINT32 const u4Transform,
    MUINT32 const u4Quality,
    MUINT32 const u4IsSOI,
    MUINT32 const eCodecType,
    MUINT32 const u4TimeoutMs//,
//    MUINT32 &u4EncSize
)
{
    MBOOL ret = MTRUE;
    if (checkIfNeedImgTransform(pSrcBufInfo, pDstBufInfo, rROI, u4Transform))
    {
        //
        IImageBuffer *pImageBuff;
        int format = pSrcBufInfo->getImgFormat();
        MUINT32 u4AlignedWidth = pDstBufInfo->getImgSize().w;
        MUINT32 u4AlignedHeight = pDstBufInfo->getImgSize().h;

        if( !isSupportedFormat((NSCam::EImageFormat)format) )
        {
            MY_LOGW("use yuy2 for jpeg encode");
            // default format
            format = eImgFmt_YUY2;
        }

        {
            // Jpeg code width/height should be aligned
            MUINT32 w_align, h_align;
            getAlignment(format, &w_align, &h_align);

            u4AlignedWidth = (~(w_align-1)) & ((w_align-1) + u4AlignedWidth);
            u4AlignedHeight = (~(h_align-1)) & ((h_align-1) + u4AlignedHeight);
            MY_LOGW("[encode] Ori (width, height) = (%d, %d), Aligned (width, height) = (%d, %d)",
                    pSrcBufInfo->getImgSize().w, pSrcBufInfo->getImgSize().h,
                    u4AlignedWidth, u4AlignedHeight);
        }

        if( !allocYuvMem(&pImageBuff,
                         MSize(u4AlignedWidth, u4AlignedHeight), format) )
        {
            return MFALSE;
        }

        //
        IImageTransform *pImgTransform = IImageTransform::createInstance();

        ret = pImageBuff->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_WRITE|eBUFFER_USAGE_SW_MASK);

        ret = pImageBuff->syncCache(eCACHECTRL_INVALID);

        ret = ret && pImgTransform->execute(pSrcBufInfo, pImageBuff, NULL, rROI, u4Transform, u4TimeoutMs);  //10s timeout

        if (ret && mu4DumpFlag)
        {
            char fileName[256] = {0};
            sprintf(fileName, "/%s/trans_%dx%d.yuv", MEDIA_PATH, u4AlignedWidth, u4AlignedHeight);
            pImageBuff->saveToFile(fileName);
        }

        pImgTransform->destroyInstance();
        // (2). Encode
        ret = ret && encode(pImageBuff, pDstBufInfo,
                            u4Quality, u4IsSOI, eCodecType, u4TimeoutMs);

        pImageBuff->unlockBuf(LOG_TAG);

        freeYuvMem(&pImageBuff);
        //
    }
    else
    {
        ret = encode(pSrcBufInfo, pDstBufInfo, u4Quality, u4IsSOI, eCodecType, u4TimeoutMs/*, u4EncSize*/);
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegCodec::
encode(
    IImageBuffer const *pSrcBufInfo,
    IImageBuffer const *pDstBufInfo,
    MUINT32 const u4Quality,
    MUINT32 const u4IsSOI,
    MUINT32 const eCodecType,
    MUINT32 const u4TimeoutMs//,
    //MUINT32 &u4EncSize
)
{
    FUNCTION_LOG_START;
    //MtkCamUtils::CamProfile profile("encode", "JpegCodec");
    MBOOL ret = MTRUE;
#ifdef JPEG_HAL_SUPPORT
    JpgEncHal* pJpgEncoder = new JpgEncHal();
    CHECK_OBJECT(pJpgEncoder);

    MUINT32 planenum = 0;
    MUINT32 encfmt = JpgEncHal:: kENC_YUY2_Format;
    MUINT32 plane[3] = {0};
    JpgEncHal::EncLockType lockType = JpgEncHal::JPEG_ENC_LOCK_HW_FIRST;
    switch(eCodecType)
    {
        case JPEGENC_HW_FIRST:
            lockType = JpgEncHal::JPEG_ENC_LOCK_HW_FIRST;
            break;
        case JPEGENC_SW:
            lockType = JpgEncHal::JPEG_ENC_LOCK_SW_ONLY;
            break;
        default:
            break;
    }

    // (1). Lock
    if(!pJpgEncoder->LevelLock(lockType))
    {
        MY_LOGE("can't lock jpeg resource");
        goto EXIT;
    }

    if( !mapToJpgFmt(pSrcBufInfo->getImgFormat(), &encfmt, &planenum) )
    {
        MY_LOGE("map to jpeg fmt failed: 0x%x", pSrcBufInfo->getImgFormat());
        goto EXIT;
    }

    // (2). size, format, addr
    if ( planenum == 1 )
    {
        ret = pJpgEncoder->setEncSize(pSrcBufInfo->getImgSize().w,
                                      pSrcBufInfo->getImgSize().h,
                                      static_cast<JpgEncHal::EncFormat>(encfmt))
            && pJpgEncoder->setSrcAddr((void *)pSrcBufInfo->getBufVA(0), (void *)NULL)
            && pJpgEncoder->setSrcBufSize(pJpgEncoder->getSrcBufMinStride(),
                                          pSrcBufInfo->getBufSizeInBytes(0),
                                          0);

    }
    else if ( planenum == 2 )
    {
        ret = pJpgEncoder->setEncSize(pSrcBufInfo->getImgSize().w,
                                      pSrcBufInfo->getImgSize().h,
                                      static_cast<JpgEncHal::EncFormat>(encfmt))
            && pJpgEncoder->setSrcAddr((void *)pSrcBufInfo->getBufVA(0),
                                       (void *)pSrcBufInfo->getBufVA(1))
            && pJpgEncoder->setSrcBufSize(pJpgEncoder->getSrcBufMinStride(),
                                          pSrcBufInfo->getBufSizeInBytes(0),
                                          pSrcBufInfo->getBufSizeInBytes(1));
    }
    else if ( planenum == 3 )
    {
        ret = pJpgEncoder->setEncSize(pSrcBufInfo->getImgSize().w,
                                      pSrcBufInfo->getImgSize().h,
                                      static_cast<JpgEncHal::EncFormat>(encfmt))
            // workaround for YV12
            && pJpgEncoder->setSrcAddr((void *)pSrcBufInfo->getBufVA(0),
                                       (void *)pSrcBufInfo->getBufVA(2),
                                       (void *)pSrcBufInfo->getBufVA(1))
            && pJpgEncoder->setSrcBufSize(pJpgEncoder->getSrcBufMinStride(),
                                          pSrcBufInfo->getBufSizeInBytes(0),
                                          pSrcBufInfo->getBufSizeInBytes(1),
                                          pSrcBufInfo->getBufSizeInBytes(2));
    }
    else
    {
        MY_LOGE("Not support image format:0x%x", (NSCam::EImageFormat)pSrcBufInfo->getImgFormat());
        goto EXIT;
    }

    // (3). set quality
    ret = ret && pJpgEncoder->setQuality(u4Quality)
    // (4). dst addr, size
          && pJpgEncoder->setDstPAddr((void *)pDstBufInfo->getBufPA(0))
          && pJpgEncoder->setDstAddr((void *)pDstBufInfo->getBufVA(0))
          && pJpgEncoder->setDstSize(pDstBufInfo->getBufSizeInBytes(0));

    // (5). ion mode
    {
        MINT32 srcfd_count = pSrcBufInfo->getFDCount();
        MINT32 dstfd_count = pDstBufInfo->getFDCount();
        // set Ion mode
        // jpeg encoder doesn't support 3 plane yet
        if( planenum < 3 && srcfd_count && dstfd_count )
        {
            MINT srcfd0 = -1, srcfd1 = -1;

            //MY_LOGD("ion mode(1)");
            pJpgEncoder->setIonMode(1);

            if ( planenum == 2 )
            {
                if( srcfd_count == 1 )
                    srcfd0 = srcfd1 = pSrcBufInfo->getFD(0);
                else
                {
                    srcfd0 = pSrcBufInfo->getFD(0);
                    srcfd1 = pSrcBufInfo->getFD(1);
                }
            }
            else
            {
                srcfd0 = pSrcBufInfo->getFD(0);
            }

            MY_LOGD("set src fd %d, %d", srcfd0, srcfd1);
            pJpgEncoder->setSrcFD(srcfd0, srcfd1);
            pJpgEncoder->setDstFD(pDstBufInfo->getFD(0));
        }
        else if ( planenum == 3 || (!srcfd_count && !dstfd_count) )
        {
            //MY_LOGD("ion mode(0)");
            pJpgEncoder->setIonMode(0);
        }
        else
        {
            ret = MFALSE;
            MY_LOGE("not support yet src fd count %d, dst fd count %d", srcfd_count, dstfd_count);
        }
    }

    // (6). set SOI
    pJpgEncoder->enableSOI((u4IsSOI > 0) ? 1 : 0);

    // (7).  Start

    MUINT32 u4EncSize;
    if (pJpgEncoder->start(&u4EncSize))
    {
        MY_LOGD("Jpeg encode done, size = %d", u4EncSize);
        ret = (const_cast<IImageBuffer*>(pDstBufInfo))->setBitstreamSize(u4EncSize);
    }
    else
    {
        MY_LOGE("encode start failed");
        ret = MFALSE;
        pJpgEncoder->unlock();
        goto EXIT;
    }

    pJpgEncoder->unlock();
    //profile.print();

EXIT:

    if(!ret)
    {
        //dump settings
        MY_LOGE("lock type 0x%x", eCodecType);
        MUINT32 plane = queryPlaneCount(pSrcBufInfo->getImgFormat());
        MY_LOGE(" fmt 0x%x, wxh %dx%d, va 0x%x/0x%x, pa 0x%x/0x%x fd %d/%d, size %d/%d",
                pSrcBufInfo->getImgFormat(),
                pSrcBufInfo->getImgSize().w       , pSrcBufInfo->getImgSize().h                       ,
                pSrcBufInfo->getBufVA(0)          , plane>1 ? pSrcBufInfo->getBufVA(1) : 0            ,
                pSrcBufInfo->getBufPA(0)          , plane>1 ? pSrcBufInfo->getBufPA(1) : 0            ,
                pSrcBufInfo->getFD(0)             , plane>1 ? pSrcBufInfo->getFD(1) : 0               ,
                pSrcBufInfo->getBufSizeInBytes(0) , plane>1 ? pSrcBufInfo->getBufSizeInBytes(1) : 0);
        MY_LOGE("quality %u, dstV/P 0x%x/0x%x, size %d, soi %d",
                 u4Quality,
                 pDstBufInfo->getBufVA(0), pDstBufInfo->getBufPA(0),
                 pDstBufInfo->getBufSizeInBytes(0),
                 u4IsSOI );
    }
    delete pJpgEncoder;

    FUNCTION_LOG_END;
#else
#warning [jpeg hal not ready yet]
#endif
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegCodec::
isSupportedFormat(EImageFormat const eFmt)
{
    MY_LOGD("Format:0x%x", eFmt);

    switch(eFmt)
    {
        case eImgFmt_YUY2:
        //case eImgFmt_UYVY:
        case eImgFmt_NV12:
        case eImgFmt_NV21:
        //case eImgFmt_YV12:
            return MTRUE;
        default:
            break;
    }
    MY_LOGD("not supported fmt 0x%x", eFmt);
    return MFALSE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
JpegCodec::
getAlignment(MINT32 const eFmt, MUINT32* width, MUINT32* height)
{
    // width/heigh alignment should in 2^k form
    switch( eFmt )
    {
        case eImgFmt_YUY2:
#if 0
        case eImgFmt_UYVY: //sw
#endif
        case eImgFmt_NV12:
        case eImgFmt_NV21:
            *width = 16;
            *height = 16;
            return MTRUE;
#if 0
        case eImgFmt_YV12: //sw, no limitation
            *width = 1;
            *height = 1;
            return MTRUE;
#endif
        default:
            //jpeg not support
            break;
    }
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegCodec::
mapToJpgFmt(MINT32 imgfmt, MUINT32* jpgfmt, MUINT32* planenum)
{
    switch(imgfmt)
    {
        case eImgFmt_YUY2:
            *planenum = 1;
            *jpgfmt = JpgEncHal::kENC_YUY2_Format;
            break;
#if 0
        case eImgFmt_UYVY:
            *planenum = 1;
            *jpgfmt = JpgEncHal::kENC_UYVY_Format;
            break;
#endif
        case eImgFmt_NV12:
            *planenum = 2;
            *jpgfmt = JpgEncHal::kENC_NV12_Format;
            break;
        case eImgFmt_NV21:
            *planenum = 2;
            *jpgfmt = JpgEncHal::kENC_NV21_Format;
            break;
#if 0
        case eImgFmt_YV12:
            *planenum = 3;
            *jpgfmt = JpgEncHal::kENC_YV12_Format;
            break;
#endif
        default:
            return MFALSE;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegCodec::
checkIfNeedImgTransform(
    IImageBuffer const *pSrcBufInfo,
    IImageBuffer const * pDstBufInfo,
    MRect const rROI,
    MUINT32 const u4Transform

)
{
    // resize
    if (pDstBufInfo->getImgSize() != pSrcBufInfo->getImgSize() )
    {
        MY_LOGD("Resize src =(%d,%d), dst=(%d,%d)",
                 pSrcBufInfo->getImgSize().w, pSrcBufInfo->getImgSize().h,
                 pDstBufInfo->getImgSize().w, pDstBufInfo->getImgSize().h);
        return MTRUE;
    }

    // check if src meets alignment
    {
        MUINT32 w_align, h_align;
        if( getAlignment(pSrcBufInfo->getImgFormat(), &w_align, &h_align) )
        {
            if (((pSrcBufInfo->getImgSize().w & (w_align-1) ) != 0) ||
                    ((pSrcBufInfo->getImgSize().h & (h_align-1) ) != 0))
            {
                MY_LOGD("src fmt 0x%x, width/height not aligh to %dx/%dx, src =(%d, %d)",
                        pSrcBufInfo->getImgFormat(),
                        w_align,
                        h_align,
                        pSrcBufInfo->getImgSize().w,
                        pSrcBufInfo->getImgSize().h
                        );
                return MTRUE;
            }
        }
    }

    // roi
    if (rROI.p.x != 0 || rROI.p.y != 0
        || rROI.s.w != pSrcBufInfo->getImgSize().w
        || rROI.s.h != pSrcBufInfo->getImgSize().h)
    {
        MY_LOGD("Crop , roi = (%d, %d, %d, %d)", rROI.p.x, rROI.p.y,
                                                 rROI.s.w, rROI.s.h);
        return MTRUE;
    }
    // image transform
    if (0 != u4Transform)
    {
        MY_LOGD("u4Transform: %d", u4Transform);
        return MTRUE;
    }
    // JPEG format but source format not support
    if (!isSupportedFormat((NSCam::EImageFormat)pSrcBufInfo->getImgFormat()))
    {
        MY_LOGD("Not JPEG codec support fmt:0x%x", (NSCam::EImageFormat)pSrcBufInfo->getImgFormat());
        return MTRUE;
    }
    //
    MY_LOGV("No need to do image transform");

    return MFALSE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
JpegCodec::
allocYuvMem(IImageBuffer **pBuf, MSize const & imgSize, int const format)
{
    MUINT32 bufStridesInBytes[3] = {0};

using namespace NSCam::Utils::Format;
    for (int i = 0; i < (int)queryPlaneCount(format); i++)
    {
        bufStridesInBytes[i] =
            (queryPlaneWidthInPixels(format,i, imgSize.w) * queryPlaneBitsPerPixel(format,i)) >> 3;
    }

    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    //
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    IImageBufferAllocator::ImgParam imgParam(
        format, imgSize, bufStridesInBytes, bufBoundaryInBytes, Utils::Format::queryPlaneCount(format)
    );

    *pBuf = allocator->alloc_ion("allocYuvBuf", imgParam);
    if  ( *pBuf == 0 )
    {
        CAM_LOGE("NULL YUV Buffer\n");
        return  MFALSE;
    }
    //
    MY_LOGD("<YUV> ImgBitsPerPixel:%d BufSizeInBytes:%d", (*pBuf)->getImgBitsPerPixel(), (*pBuf)->getBufSizeInBytes(0));
    return  MTRUE;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
JpegCodec::
freeYuvMem(IImageBuffer **pBuf)
{
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    allocator->free(*pBuf);
    *pBuf = NULL;
    return  MTRUE;
}


////////////////////////////////////////////////////////////////////////////////

