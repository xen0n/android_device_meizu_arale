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
#define LOG_TAG "halUTILITY"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/featureio/IHal3A.h>
#include <mtkcam/v1/camutils/CamInfo.h>
#include <mtkcam/v1/IParamsManager.h>
#include <utility_hal.h>
#include "enc/jpeg_hal.h"

#include "mtkcam/exif/IBaseCamExif.h"
#include "mtkcam/exif/CamExif.h"

using namespace android;
using namespace MtkCamUtils;
using namespace NS3A;
using namespace NSCam;

static inline MUINT32 _align_mcu(MUINT32 x)
{
    const MUINT32 MCU_ALIGN_LENGTH = 16;
    return (((x) + (MCU_ALIGN_LENGTH - 1)) / MCU_ALIGN_LENGTH) * MCU_ALIGN_LENGTH;
}

/*******************************************************************************
*
********************************************************************************/

static halUTILITYBase *pHalUTILITY = NULL;
/*******************************************************************************
*
********************************************************************************/
halUTILITYBase*
halUTILITY::
getInstance()
{
    CAM_LOGD("[halUTILITY] getInstance \n");
    if (pHalUTILITY == NULL) {
        pHalUTILITY = new halUTILITY();
    }
    return pHalUTILITY;
}

/*******************************************************************************
*
********************************************************************************/
void
halUTILITY::
destroyInstance()
{
    if (pHalUTILITY) {
        delete pHalUTILITY;
    }
    pHalUTILITY = NULL;
}

/*******************************************************************************
*
********************************************************************************/
halUTILITY::halUTILITY()
{
}


halUTILITY::~halUTILITY()
{
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
halUTILITY::
makeExifHeader(MUINT32 const u4CamMode,
                     MUINT8* const puThumbBuf,
                       MUINT32 const u4ThumbSize,
                       MUINT8* puExifBuf,
                       MUINT32 &u4FinalExifSize,
                       MUINT32 const Width,
                       MUINT32 const Height,
                       MUINT32 u4ImgIndex,
                       MUINT32 u4GroupId,
                       MVOID* pParamsMgr)
{
    //
    CAM_LOGD("+ (u4CamMode, puThumbBuf, u4ThumbSize, puExifBuf) = (%d, %p, %d, %p)",
                            u4CamMode,  puThumbBuf, u4ThumbSize, puExifBuf);
    sp<IParamsManager> mpParamsMgr = *((sp<IParamsManager>*)pParamsMgr);
    if (u4ThumbSize > 63 * 1024)
    {
        CAM_LOGE("The thumbnail size is large than 63K, the exif header will be broken");
    }
    bool ret = true;
    uint32_t u4App1HeaderSize = 0;
    uint32_t u4AppnHeaderSize = 0;

    uint32_t exifHeaderSize = 0;
    CamExif rCamExif;
    CamExifParam rExifParam;
    CamDbgParam rDbgParam;

    // ExifParam (for Gps)
    if (! mpParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).isEmpty() && !mpParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).isEmpty())
    {
        rExifParam.u4GpsIsOn = 1;
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSLatitude), mpParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).string(), mpParamsMgr->getStr(CameraParameters::KEY_GPS_LATITUDE).length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSLongitude), mpParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).string(), mpParamsMgr->getStr(CameraParameters::KEY_GPS_LONGITUDE).length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSTimeStamp), mpParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP).string(), mpParamsMgr->getStr(CameraParameters::KEY_GPS_TIMESTAMP).length());
        ::strncpy(reinterpret_cast<char*>(rExifParam.uGPSProcessingMethod), mpParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD).string(), mpParamsMgr->getStr(CameraParameters::KEY_GPS_PROCESSING_METHOD).length());
        rExifParam.u4GPSAltitude = ::atoi(mpParamsMgr->getStr(CameraParameters::KEY_GPS_ALTITUDE).string());
    }

    rExifParam.u4Orientation = mpParamsMgr->getInt(CameraParameters::KEY_ROTATION);
    rExifParam.u4ZoomRatio = mpParamsMgr->getZoomRatio();
    //
    rExifParam.u4ImgIndex = u4ImgIndex;
    rExifParam.u4GroupId = u4GroupId;
    //
    //! CamDbgParam (for camMode, shotMode)
    rDbgParam.u4CamMode = u4CamMode;
    //
    rCamExif.init(rExifParam,  rDbgParam);
    CAM_LOGD("3A get exif");
    IHal3A* p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1,mpParamsMgr->getOpenId(),"PreviewFeature");
    p3AHal->set3AEXIFInfo(&rCamExif);

    //
    // the bitstream already rotated. it need to swap the width/height
    if (90 == rExifParam.u4Orientation || 270 == rExifParam.u4Orientation)
    {
        rCamExif.makeExifApp1(Height,  Width, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
    }
    else
    {
        rCamExif.makeExifApp1(Width, Height, u4ThumbSize, puExifBuf,  &u4App1HeaderSize);
    }
    // copy thumbnail image after APP1
    MUINT8 *pdest = puExifBuf + u4App1HeaderSize;
    ::memcpy(pdest, puThumbBuf, u4ThumbSize) ;
    //

    pdest = puExifBuf + u4App1HeaderSize + u4ThumbSize;
    //
    rCamExif.appendDebugExif(pdest, &u4AppnHeaderSize);
    rCamExif.uninit();
    p3AHal->destroyInstance("MTKPanorama");
    u4FinalExifSize = u4App1HeaderSize + u4ThumbSize + u4AppnHeaderSize;

    CAM_LOGD("- (app1Size, appnSize, exifSize) = (%d, %d, %d)",
                          u4App1HeaderSize, u4AppnHeaderSize, u4FinalExifSize);
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
halUTILITY::
createJpegImg(NSCamHW::ImgBufInfo const & rSrcBufInfo
      , MUINT32 quality
      , bool fIsAddSOI
      , NSCamHW::ImgBufInfo const & rDstBufInfo
      , MUINT32 & u4EncSize)
{
    MBOOL ret = MTRUE;
    // (0). debug
    CAM_LOGD("[createJpegImg] - rSrcImgBufInfo.u4BufVA=0x%x", rSrcBufInfo.u4BufVA);
    CAM_LOGD("[createJpegImg] - rSrcImgBufInfo.eImgFmt=%d", rSrcBufInfo.eImgFmt);
    CAM_LOGD("[createJpegImg] - rSrcImgBufInfo.u4ImgWidth=%d", rSrcBufInfo.u4ImgWidth);
    CAM_LOGD("[createJpegImg] - rSrcImgBufInfo.u4ImgHeight=%d", rSrcBufInfo.u4ImgHeight);
    CAM_LOGD("[createJpegImg] - jpgQuality=%d", quality);
    //
    // (1). Create Instance
    JpgEncHal* pJpgEncoder = new JpgEncHal();
    // (1). Lock
    if(!pJpgEncoder->lock())
    {
        CAM_LOGE("can't lock jpeg resource");
        goto EXIT;
    }
    // (2). size, format, addr
    if (eImgFmt_YUY2 == rSrcBufInfo.eImgFmt)
    {
        CAM_LOGD("jpeg source YUY2");
        pJpgEncoder->setEncSize(rSrcBufInfo.u4ImgWidth, rSrcBufInfo.u4ImgHeight,
                                JpgEncHal:: kENC_YUY2_Format);
        pJpgEncoder->setSrcAddr((void *)rSrcBufInfo.u4BufVA, (void *)NULL);
        pJpgEncoder->setSrcBufSize(pJpgEncoder->getSrcBufMinStride() ,rSrcBufInfo.u4BufSize, 0);
    }
    else if (eImgFmt_NV21 == rSrcBufInfo.eImgFmt)
    {
        CAM_LOGD("jpeg source NV21");
        pJpgEncoder->setEncSize(rSrcBufInfo.u4ImgWidth, rSrcBufInfo.u4ImgHeight,
                                JpgEncHal:: kENC_NV21_Format);
        pJpgEncoder->setSrcAddr((void *)rSrcBufInfo.u4BufVA, (void *)(rSrcBufInfo.u4BufVA + rSrcBufInfo.u4ImgWidth * rSrcBufInfo.u4ImgHeight));
        pJpgEncoder->setSrcBufSize(pJpgEncoder->getSrcBufMinStride(), _align_mcu(rSrcBufInfo.u4ImgWidth) * _align_mcu(rSrcBufInfo.u4ImgHeight),
                                                 _align_mcu(rSrcBufInfo.u4ImgWidth) * _align_mcu(rSrcBufInfo.u4ImgHeight) / 2);
    }
    else
    {
        CAM_LOGE("Not support image format:0x%x", rSrcBufInfo.eImgFmt);
        goto EXIT;
    }
    // (3). set quality
    pJpgEncoder->setQuality(quality);
    // (4). dst addr, size
    pJpgEncoder->setDstAddr((void *)rDstBufInfo.u4BufVA);
    pJpgEncoder->setDstSize(rDstBufInfo.u4BufSize);
    // (6). set SOI
    pJpgEncoder->enableSOI((fIsAddSOI > 0) ? 1 : 0);
    // (7). ION mode
    if ( rSrcBufInfo.i4MemID > 0 )
    {
        pJpgEncoder->setIonMode(1);
        pJpgEncoder->setSrcFD(rSrcBufInfo.i4MemID, rSrcBufInfo.i4MemID);
        pJpgEncoder->setDstFD(rDstBufInfo.i4MemID);
    }

    // (8).  Start
    if (pJpgEncoder->start(&u4EncSize))
    {
        CAM_LOGD("Jpeg encode done, size = %d", u4EncSize);
        ret = MTRUE;
    }
    else
    {
        pJpgEncoder->unlock();
        goto EXIT;
    }

    pJpgEncoder->unlock();

EXIT:
    delete pJpgEncoder;

    CAM_LOGD("[init] - X. ret: %d.", ret);
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
halUTILITY::
createJpegImgWithThumbnail(NSCamHW::ImgBufInfo const &rYuvImgBufInfo, IMEM_BUF_INFO jpgBuff, MUINT32 &u4JpegSize, MUINT32 quality)
{
    MBOOL ret = MTRUE;
    MUINT32 stride[3];
    CAM_LOGD("[createJpegImgWithThumbnail] in");
    CAM_LOGD("jpgBuff.size %d jpgBuff.virtAddr 0x%x  jpgBuff.phyAddr 0x%x", jpgBuff.size, (MUINT32)jpgBuff.virtAddr,(MUINT32)jpgBuff.phyAddr);

    NSCamHW::ImgInfo    rJpegImgInfo(eImgFmt_JPEG, rYuvImgBufInfo.u4ImgWidth, rYuvImgBufInfo.u4ImgHeight);
    NSCamHW::BufInfo    rJpegBufInfo((int)jpgBuff.size,(MUINT32)jpgBuff.virtAddr, (MUINT32)jpgBuff.phyAddr, jpgBuff.memID);
    NSCamHW::ImgBufInfo   rJpegImgBufInfo(rJpegImgInfo, rJpegBufInfo, stride);

    ret = createJpegImg(rYuvImgBufInfo, quality, MFALSE, rJpegImgBufInfo, u4JpegSize);

    CAM_LOGD("[createJpegImgWithThumbnail] out jpgBuff.size %d ", jpgBuff.size);
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
halUTILITY::
createPanoJpegImg(MVOID*   pMPImageInfo
                  , MVOID* Srcbuf
                  , int u4SrcWidth
                  , int u4SrcHeight
                  , MUINT32     Jpgbufsize
                  , MINT32      JpgbufmemID
                  , MUINT32     JpgbufvirtAddr
                  , MUINT32     JpgbufphyAddr
                  , MUINT32     &u4JpegSize
                  , MUINT32     quality)
{
    CAM_LOGD("[createPanoJpegImg] in");
    MBOOL ret = MTRUE;
    MUINT32     u4Stride[3];
    u4Stride[0] = u4SrcWidth;
    u4Stride[1] = u4SrcWidth >> 1;
    u4Stride[2] = u4SrcWidth >> 1;

    //IMEM_BUF_INFO* srcBuff=(IMEM_BUF_INFO*)Srcbuf;
    IMEM_BUF_INFO srcBuff;
    srcBuff.size = (u4SrcWidth * u4SrcHeight * 2);
    srcBuff.virtAddr = (MUINTPTR)Srcbuf;
    srcBuff.memID = -1;

    IMEM_BUF_INFO jpgBuff;

    NSCamHW::ImgInfo    rYuvImgInfo(eImgFmt_NV21, u4SrcWidth , u4SrcHeight);
    NSCamHW::BufInfo    rYuvBufInfo(srcBuff.size, (MUINT32)srcBuff.virtAddr, 0, srcBuff.memID);
    NSCamHW::ImgBufInfo   rYuvImgBufInfo(rYuvImgInfo, rYuvBufInfo, u4Stride);
    jpgBuff.size=Jpgbufsize;
    jpgBuff.virtAddr=JpgbufvirtAddr;
    jpgBuff.phyAddr=JpgbufphyAddr;
    jpgBuff.memID=JpgbufmemID;
    ret = createJpegImgWithThumbnail(rYuvImgBufInfo,jpgBuff,u4JpegSize,quality);

    MUINTPTR *pImageInfo = (MUINTPTR*)pMPImageInfo;

    *pImageInfo = srcBuff.virtAddr;
    CAM_LOGD("[createPanoJpegImg] out addr 0x%x", srcBuff.virtAddr);
    return ret;
}
