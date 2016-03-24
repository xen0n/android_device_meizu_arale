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
#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_IJPEG_CODEC_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_IJPEG_CODEC_H_


//using namespace NSCamHW;
/*******************************************************************************
*
********************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSSImager {
////////////////////////////////////////////////////////////////////////////////



/*******************************************************************************
* Interface of Jpeg Codec
********************************************************************************/
class IJpegCodec
{
protected:  ////    Constructor/Destructor.
    virtual         ~IJpegCodec() {}

public:     ////    Attributes
    virtual MINT32      getLastErrorCode() const = 0;

public:     ////    Instantiation.
    static IJpegCodec* createInstance();
    virtual MVOID   destroyInstance() = 0;

public:     ////    Operations.
    /*
     *  Jpeg Codec , the functionality is used to encode jpeg
     *
     *  Params:
     *      pSrcBufInfo
     *      [I] The image buffer info of the input image
     *
     *
     *      pDstBufInfo
     *      [I] The image buffer info of the output image
     *
     *      rROI
     *      [I] The crop ROI of the source image
     *
     *      u4Transform
     *      [I] The image transform of the image
     *
     *      u4Quality
     *      [I] The Jpeg quality 1~100
     *
     *      u4IsSOI
     *      [I] Is include SOI
     *
     *      eCodecType
     *      [I] one of JPEGENC_HW_FIRST or JPEGENC_HW_FORCE
     *          should be set
     *          If JPEGENC_HW_FIRST is set, SImager will check
     *          if hw codec available and then use it. If not
     *          available, sw codec is used instead.
     *          Once the JPEGENC_HW_FORCE is set, it will wait
     *          for hw codec ready until execution timeout.
     *
     *      u4EncSize
     *      [O] Jpeg output bitstream size
     *
     *  Return:
     *      MTRUE indicates success; MFALSE indicates failure, and an error code
     *      can be retrived by getLastErrorCode().
     */
    virtual  MBOOL    encode(
                          IImageBuffer const *pSrcBufInfo,
                          IImageBuffer const *pDstBufInfo,
                          MRect const rROI,
                          MUINT32 const u4Transform,
                          MUINT32 const u4Quality,
                          MUINT32 const u4IsSOI,
                          MUINT32 const eCodecType,
                          MUINT32 const u4TimeoutMs
                      ) = 0;

};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSSImager
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //  _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_IJPEG_CODEC_H_

