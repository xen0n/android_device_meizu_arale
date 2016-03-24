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
#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_H_

//
#include <utils/Vector.h>

//
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>

/*******************************************************************************
*
********************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSSImager {

////////////////////////////////////////////////////////////////////////////////



/*******************************************************************************
*
********************************************************************************/
class SImager
{
protected:

public:     ////    Constructor/Destructor.
    SImager(IImageBuffer const * pImageBuffer);
    virtual ~SImager() {};

public:     ////    Instantiation.

public:      //// attributes
#if 0
    virtual MUINT32  getJpegSize() {return mu4JpegSize; };
#endif

public:     ////    Operations.
    virtual MBOOL    execute(MUINT32 u4TimeoutMs = 0xFFFFFFFF);


public:     ////    Settings.
    virtual MBOOL   setTargetImgBuffer(IImageBuffer const *pImgBuffer);
    virtual MBOOL   setTransform(MUINT32 const u4Transform);
    virtual MBOOL   setCropROI(MRect const rCropRect);
    virtual MBOOL   setEncodeParam(MUINT32 const &u4IsSOI,
                                   MUINT32 const &u4Quality,
                                   MUINT32 const eCodecType);
#if 0
    virtual MBOOL   setFormat(EImageFormat const eFormat);
    virtual MBOOL   setResize(MSize const rSize);
#endif

public:     ////    Attributes
    virtual MINT32      getLastErrorCode() const    { return mi4ErrorCode; }

private:   ////
    MINT32              mi4ErrorCode;
    MVOID*              mpCbUser;           //  Callback user.
private:   ////
    //MUINT32     mu4JpegSize;
    IImageBuffer const *mpSrcImgBuf;
    IImageBuffer const *mpDstImgBuf;
    MUINT32 mu4Transform;
    MRect mCropRect;
#if 0
    MSize mSize;
    EImageFormat mFormat;
#endif
    MUINT32 mu4Quality;
    MUINT32 mu4IsSOI;
    MUINT32 meCodecType;


private:     ////
    MBOOL    isSupportedSrcFormat(EImageFormat const eFmt);
    //
    MBOOL    encode(
                 IImageBuffer const *pSrcBufInfo,
                 IImageBuffer const *pDstBufInfo,
                 MRect const rROI,
                 /*MSize const rSize, */
                 MUINT32 const u4Transform,
                 MUINT32 const u4Quality,
                 MUINT32 const u4IsSOI,
                 MUINT32 const eCodecType,
                 MUINT32 const u4TimeoutMs//,
                 //MUINT32 &u4EncSize
              );
    //
    MBOOL    imgTransform(
                 IImageBuffer const *pSrcBufInfo,
                 IImageBuffer const * pDstBufInfo,
                 MRect const rROI,
                 MUINT32 const u4Transform,
                 MUINT32 const u4TimeOutInMs
             );
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSSImager
};  //namespace NSIoPipe
};  //namespace NSCam

#endif  //  _MTK_PLATFORM_HARDWARE_MTKCAM_CORE_IOPIPE_SIMAGER_H_

