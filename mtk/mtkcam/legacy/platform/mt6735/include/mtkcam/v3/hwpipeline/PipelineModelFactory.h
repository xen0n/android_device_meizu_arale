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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINEMODELFACTORY_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINEMODELFACTORY_H_
//
#include <mtkcam/v3/pipeline/IPipelineModel.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
struct  ConfigurePipeline
{
    typedef android::KeyedVector<
                StreamId_T,
                android::sp<IImageStreamInfo>
                                >   ImageStreamsT;

    typedef android::KeyedVector<
                StreamId_T,
                android::sp<IMetaStreamInfo>
                                >   MetaStreamsT;

    typedef android::KeyedVector<
                StreamId_T,
                MINT64
                                >   MinFrameDurationT;

    typedef android::KeyedVector<
                StreamId_T,
                MINT64
                                >   StallFrameDurationT;

    struct  Params
    {
        /**
         * @param[in] open Id.
         *  The caller must promise its value.
         */
        MINT32                      openId;

        /**
         * @param[in] A pointer to App image streams to configure.
         *  The caller must promise its initial value.
         */
        ImageStreamsT const*        pvImageStreams;

        /**
         * @param[in] A pointer to App image streams min frame duration to configure.
         *  The caller must promise its initial value.
         */
        MinFrameDurationT const*    pvMinFrameDuration;

        /**
         * @param[in] A pointer to App image streams stall frame duration to configure.
         *  The caller must promise its initial value.
         */
        StallFrameDurationT const*  pvStallFrameDuration;

        /**
         * @param[in] A pointer to App meta streams to configure.
         *  The caller must promise its initial value.
         */
        MetaStreamsT const*         pvMetaStreams;

        /**
         * @param[in,out] A pipeline model.
         *  The caller must set its old pipeline to an initial value.
         *  The callee will update it.
         */
        android::sp<IPipelineModel> pPipelineModel;
    };

    /**
     * Evaluate each buffer's users.
     *
     * @param[in,out] rParams: the input/output parameters.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    MERROR                          operator()(Params& rParams);

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWPIPELINE_PIPELINEMODELFACTORY_H_

