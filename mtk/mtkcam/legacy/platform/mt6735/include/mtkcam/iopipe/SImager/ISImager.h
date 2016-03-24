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
#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_ISIMAGER_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_ISIMAGER_H_

//
#include <utils/Vector.h>

//
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>

#include <mtkcam/iopipe/SImager/ISImagerDataTypes.h>
/*******************************************************************************
*
********************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSSImager {
////////////////////////////////////////////////////////////////////////////////

/**
 * @class ISImager
 * @brief Interface of SImager (Simple Imager)
 * @detail
 *  The data path will be MEM --> MDP --> MEM (resize, format convert, rotate..etc)
 *  or MEM --> MDP --> JPEG Enc --> MEM
 *  or MEM --> Jpeg Enc --> MEM
 */
class ISImager
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////    Constructor/Destructor.
    virtual         ~ISImager() {}

public:     ////    Instantiation.
    /**
     * @brief create the ISImager instance
     *
     * @details
     *
     * @note
     *
     * @param[in] rImageBuffer: the input image buffer
     *
     * @return
     * -   The instance of the ISImager
     *
     */
    static ISImager* createInstance(IImageBuffer const * pImageBuffer);

    /**
     * @brief destroy the ISImager instance
     *
     * @details
     *
     * @note
     *
     *
     * @return
     *
     */
    virtual MVOID   destroyInstance() = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    /**
     * @brief Start to execute the image process w/ timeout
     *
     * @details
     *
     * @note Sync function, it will block until done
     *
     * @param[in] u4TimeoutMs: the time out in ms.
     *
     * @return
     * -   MTRUE indicates the notify message type is enable;
     * -   MFALSE indicates the notify message type is disable
     *
     */
    virtual MBOOL    execute(MUINT32 const u4TimeoutMs = 0xFFFFFFFF) = 0;


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    /**
     * @brief Get the last error code
     *
     * @details
     *
     * @note
     *
     *
     * @return
     * -   The error code
     *
     */
    virtual MINT32      getLastErrorCode() const = 0;

public:     ////    Settings
    /**
     * @brief Set the target image buffer
     *
     * @details
     *
     * @note
     *
     * @param[in] rImgBuffer: The buffer info of target
     *
     * @return
     * -   MTRUE indicates the notify message type is enable;
     * -   MFALSE indicates the notify message type is disable
     *
     */
    virtual MBOOL   setTargetImgBuffer(IImageBuffer const *pImgBuffer) = 0;

    /**
     * @brief Set the transformation operation of the target image
     *
     * @details
     *
     * @note
     *
     * @param[in] u4Transform: The tranformation of the iamge
     *
     * @return
     * -   MTRUE indicates the notify message type is enable;
     * -   MFALSE indicates the notify message type is disable
     *
     */
    virtual MBOOL   setTransform(MUINT32 const u4Transform) = 0;

    /**
     * @brief Set the crop ROI of the source image
     *
     * @details
     *
     * @note
     *
     * @param[in] rCropRect: The region of interest of the source image
     *
     * @return
     * -   MTRUE indicates the notify message type is enable;
     * -   MFALSE indicates the notify message type is disable
     *
     */
    virtual MBOOL   setCropROI(MRect const rCropRect) = 0;

    /**
     * @brief Set the encode param of the jpeg
     *
     * @details
     * This function only need to call if the target image is eImgFmt_JPEG
     * @note
     *
     * @param[in] u4IsSOI: Is embedded the start of image header into Jpeg
     * @param[in] u4Quality: The jpeg encode quality factory 0 ~ 100
     * @param[in] eCodecType: one of JPEGENC_HW_FIRST or JPEGENC_SW
     *                        should be set
     *                        If JPEGENC_HW_FIRST is set, SImager will check
     *                        if hw codec available and then use it. If not,
     *                        sw codec is used instead.
     *                        Once the JPEGENC_SW is set, it will use sw codec
     *                        to encode.
     *
     * @return
     * -   MTRUE indicates the notify message type is enable;
     * -   MFALSE indicates the notify message type is disable
     *
     */
    virtual MBOOL   setEncodeParam(MUINT32 const u4IsSOI,
                                   MUINT32 const u4Quality,
                                   MUINT32 const eCodecType = JPEGENC_HW_FIRST
                                   ) = 0;

public:     //// info


};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSSImager
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //  _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_ISIMAGER_H_

