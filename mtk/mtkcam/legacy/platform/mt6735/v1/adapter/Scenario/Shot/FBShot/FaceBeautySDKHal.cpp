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

#include <mtkcam/sdk/hal/IFaceBeautySDKHal.h>
#include "Facebeauty.h"


/******************************************************************************
 *
 ******************************************************************************/
using namespace NSCam;

class FaceBeautySDKHal : public IFaceBeautySDKHal
{
public:     ////                    Instantiation.

    /**
     * @brief Destroy the instance
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    virtual MVOID                   destroyInstance();

    /**
     * @brief Init the instance
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    virtual MBOOL                   init(FACE_BEAUTY_SDK_HAL_MODE eMode);

    /**
     * @brief Uninit the instance
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    virtual MVOID                   uninit();

                                    FaceBeautySDKHal() {}

private:
    virtual                         ~FaceBeautySDKHal() {}

public:

    /**
     * @brief Apply face beauty effect
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    virtual MBOOL                   apply(
                                        IImageBuffer const* SrcImgBuffer,
                                        IImageBuffer const* DstImgBuffer,
                                        FACE_BEAUTY_SDK_HAL_PARAMS const& pParam);

private:
    FACE_BEAUTY_SDK_HAL_MODE        mMode;

};


/******************************************************************************
 *
 ******************************************************************************/
IFaceBeautySDKHal*
IFaceBeautySDKHal::createInstance()
{
    return new FaceBeautySDKHal();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FaceBeautySDKHal::destroyInstance()
{
    delete this;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FaceBeautySDKHal::init(FACE_BEAUTY_SDK_HAL_MODE eMode)
{
    mMode = eMode;
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FaceBeautySDKHal::uninit()
{
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FaceBeautySDKHal::apply(IImageBuffer const* SrcImgBuffer, IImageBuffer const* DstImgBuffer, FACE_BEAUTY_SDK_HAL_PARAMS const& pParam)
{
    MBOOL ret = MFALSE;

    if ((SrcImgBuffer == NULL) ||
            (DstImgBuffer == NULL) ||
            (SrcImgBuffer->getImgFormat() != eImgFmt_I422) ||
            (SrcImgBuffer->getImgSize().w != DstImgBuffer->getImgSize().w) ||
            (SrcImgBuffer->getImgSize().h != DstImgBuffer->getImgSize().h))
    {
        goto _Exit;
    }

    //ret = CaptureFaceBeautySDK_apply(SrcImgBuffer, DstImgBuffer, &pParam);

_Exit:
    return ret;
}
