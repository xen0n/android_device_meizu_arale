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
#define LOG_TAG "MtkCam/SImager"
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
#include <mtkcam/iopipe/SImager/IImageTransform.h>
#include "IJpegCodec.h"
//
#include "SImager.h"

#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}

using namespace android;
using namespace NSCam::NSIoPipe::NSSImager;

/*******************************************************************************
*
********************************************************************************/


/*******************************************************************************
*
********************************************************************************/
SImager::
SImager(
    IImageBuffer const * pImageBuffer
)
    : mi4ErrorCode(0)
    , mpCbUser(NULL)
    , mpSrcImgBuf(pImageBuffer)
    , mpDstImgBuf(NULL)
    , mu4Transform(0)
    , mu4Quality(96)
    , mu4IsSOI(0)
{
    if (NULL != pImageBuffer)
    {
        mCropRect.p.x = 0;
        mCropRect.p.y = 0;
        mCropRect.s.w = pImageBuffer->getImgSize().w;
        mCropRect.s.h = pImageBuffer->getImgSize().h;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SImager::
execute(MUINT32 const u4TimeoutMs)
{
    FUNCTION_LOG_START;
    MBOOL ret;
    //
    if (!mpSrcImgBuf || !mpDstImgBuf)
    {
        MY_LOGE("src(0x%x), dst(0x%x)", mpSrcImgBuf, mpDstImgBuf);
        return MFALSE;
    }
    //
    if (!isSupportedSrcFormat((NSCam::EImageFormat)mpSrcImgBuf->getImgFormat()))
    {
        MY_LOGE("Unsupport source format:0x%x", mpSrcImgBuf->getImgFormat());
        return MFALSE;
    }

    MY_LOGD("(fmt, width, height) = (0x%x, %d, %d)",
            mpDstImgBuf->getImgFormat(), mpDstImgBuf->getImgSize().w, mpDstImgBuf->getImgSize().h);

    if (eImgFmt_JPEG == mpDstImgBuf->getImgFormat() )
    {
        ret = encode(mpSrcImgBuf, mpDstImgBuf, mCropRect, mu4Transform,
                    mu4Quality, mu4IsSOI, meCodecType, u4TimeoutMs);
    }
    else
    {
        ret = imgTransform(mpSrcImgBuf, mpDstImgBuf, mCropRect, mu4Transform, u4TimeoutMs);
    }

    FUNCTION_LOG_END;
     //
    return ret;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
SImager::
setTargetImgBuffer(IImageBuffer const * pImgBuffer)
{

    MY_LOGD("dst imgbuf(0x%x)", pImgBuffer);
    mpDstImgBuf = pImgBuffer;

    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SImager::
setTransform(MUINT32 const u4Transform)
{
    MY_LOGD("Ori Transform = 0x%x, Target Transfrom = 0x%x", mu4Transform, u4Transform);
    mu4Transform = u4Transform;

    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SImager::
setCropROI(MRect const rCropRect)
{
    MY_LOGD("roi (x, y, w, h) = (%d, %d, %d, %d)", rCropRect.p.x, rCropRect.p.y, rCropRect.s.w, rCropRect.s.h);
    if (rCropRect.p.x + rCropRect.s.w > mpSrcImgBuf->getImgSize().w)
    {
        MY_LOGE("Crop x + w > source width");
        return MFALSE;
    }
    else if (rCropRect.p.y + rCropRect.s.h > mpSrcImgBuf->getImgSize().h)
    {
        MY_LOGE("Crop y + h > source height");
        return MFALSE;
    }

    mCropRect = rCropRect;

    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SImager::
setEncodeParam(MUINT32 const & u4IsSOI, MUINT32 const & u4Quality, MUINT32 const eCodecType)
{
    MY_LOGD("enc param (SOI, Quality, type) = (%d, %d, %d)", u4IsSOI, u4Quality, eCodecType);
    mu4IsSOI = u4IsSOI;
    mu4Quality = u4Quality > 95 ? 95 : u4Quality;
    meCodecType = eCodecType;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SImager::
encode(
    IImageBuffer const *pSrcBufInfo,
    IImageBuffer const *pDstBufInfo,
    MRect const rROI,
    MUINT32 const u4Transform,
    MUINT32 const u4Quality,
    MUINT32 const u4IsSOI,
    MUINT32 const eCodecType,
    MUINT32 const u4TimeoutMs
    )
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    //
    IJpegCodec *pJpegCodec = IJpegCodec::createInstance();
    ret = pJpegCodec->encode(pSrcBufInfo, pDstBufInfo, rROI, u4Transform,
                             u4Quality, u4IsSOI, eCodecType, u4TimeoutMs);
    pJpegCodec->destroyInstance();
    //
    FUNCTION_LOG_END;
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
SImager::
imgTransform(
    IImageBuffer const *pSrcBufInfo,
    IImageBuffer const * pDstBufInfo,
    MRect const rROI,
    MUINT32 const u4Transform,
    MUINT32 const u4TimeOutInMs
)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    //
    IImageTransform *pImgTransform = IImageTransform::createInstance();
    ret =  pImgTransform->execute(pSrcBufInfo, pDstBufInfo, NULL, rROI, u4Transform, u4TimeOutInMs);
    pImgTransform->destroyInstance();
    //
    FUNCTION_LOG_END;
    return ret ;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
SImager::
isSupportedSrcFormat(EImageFormat const eFmt)
{
    EImageFormat eUnSupportFmtList[] = {eImgFmt_BAYER8, eImgFmt_BAYER10, eImgFmt_BAYER12,
                                       eImgFmt_NV21_BLK, eImgFmt_NV12_BLK, eImgFmt_JPEG,
                                       eImgFmt_RGB888, eImgFmt_ARGB888};
    //
    for (MUINT32 i = 0; i < sizeof(eUnSupportFmtList)/sizeof(EImageFormat) ; i++)
    {
        if (eFmt == eUnSupportFmtList[i])
        {
            return MFALSE;
        }
    }

    return MTRUE;
}



