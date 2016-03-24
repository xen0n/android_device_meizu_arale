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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P1NODE_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_P1NODE_H_

#include <mtkcam/v3/pipeline/IPipelineNode.h>
#include <mtkcam/v3/stream/IStreamInfo.h>
#include <mtkcam/v3/stream/IStreamBuffer.h>
#include <mtkcam/v3/utils/streambuf/IStreamBufferPool.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {

/******************************************************************************
 *
 ******************************************************************************/
class P1Node
    : public virtual IPipelineNode
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::Vector<android::sp<IMetaStreamInfo> >  MetaStreamInfoSetT;
    typedef android::Vector<android::sp<IImageStreamInfo> > ImageStreamInfoSetT;
    typedef NSCam::v3::Utils::IStreamBufferPool<IImageStreamBuffer> IImageStreamBufferPoolT;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    /**
     * Sensor Parameters.
     */
    struct SensorParams
    {

        MUINT       mode;
        MSize       size;
    };


    /**
     * Configure Parameters.
     */
    struct  ConfigParams
    {
        /**
         * A pointer to input meta stream info.
         */
        android::sp<IMetaStreamInfo> pInAppMeta;

        /**
         * A pointer to input meta stream info.
         */
        android::sp<IMetaStreamInfo> pInHalMeta;

        /**
         * A pointer to output meta stream info.
         */
        android::sp<IMetaStreamInfo> pOutAppMeta;

        /**
         * A pointer to output meta stream info.
         */
        android::sp<IMetaStreamInfo> pOutHalMeta;

        /**
         * A pointer to output image stream info. (resizer output port)
         */
        android::sp<IImageStreamInfo> pOutImage_resizer;

        /**
         * A pointer to output image stream info. (full ouput port)
         */
        android::Vector<android::sp<IImageStreamInfo> > pvOutImage_full;


        /**
         * A pointer to sensor parameter. Must Have.
         */
        SensorParams                 sensorParams;

        /**
         * A pointer to full output image stream info.
         */
        android::sp<IImageStreamBufferPoolT> pStreamPool_resizer;

        /**
         * A pointer to resizer output image stream info.
         */
        android::sp<IImageStreamBufferPoolT> pStreamPool_full;
    };


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    static  android::sp<P1Node>     createInstance();

    virtual MERROR                  config(ConfigParams const& rParams) = 0;

    virtual MERROR                  init(InitParams const& rParams)= 0;

public:     ////                    Operations.

    virtual void                    doNotifyCb (
                                        MINT32  _msgType,
                                        MINTPTR _ext1,
                                        MINTPTR _ext2,
                                        MINTPTR _ext3
                                    ) = 0;

};

};
};

#endif
