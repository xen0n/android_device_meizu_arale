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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINEMODEL_DEFAULT_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINEMODEL_DEFAULT_H_
//
#include "NodeId.h"
#include "StreamId.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
class PipelineModel_Default
    : public virtual IPipelineModel
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    struct  PipeConfigParams
    {
        android::sp<IMetaStreamInfo>pMeta_Control;

        android::sp<IImageStreamInfo>
                                    pImage_Raw;

        android::sp<IImageStreamInfo>
                                    pImage_Jpeg_Stall;

        android::Vector <
            android::sp<IImageStreamInfo>
                        >           vImage_Yuv_NonStall;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.
    static  PipelineModel_Default*  create(MINT32 const openId);

public:     ////                    Attributes.
    static  char const*             magicName() { return "PipelineModel_Default"; }

public:     ////                    Operations.
    /**
     * Configure.
     *
     * @param[in] rAppParams
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                  configure(
                                        PipeConfigParams const& rConfigParams
                                    )                                       = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINEMODEL_DEFAULT_H_

