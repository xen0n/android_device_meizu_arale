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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINENODEMAPCONTROL_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINENODEMAPCONTROL_H_
//
#include "IPipelineNode.h"
#include <mtkcam/v3/utils/streaminfo/IStreamInfoSetControl.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/**
 * An interface of Pipeline node map (key:NodeId_T, value:NodePtrT).
 */
class IPipelineNodeMapControl
    : public virtual IPipelineNodeMap
{
public:     ////                            Definitions.
    typedef IPipelineNodeMapControl         ThisT;
    typedef Utils::IStreamInfoSetControl    IStreamSetT;
    typedef android::sp<IStreamSetT>        IStreamSetPtr;
    typedef android::sp<IStreamSetT const>  IStreamSetPtr_CONST;

    struct  INode
        : public virtual android::RefBase
    {
    public:     ////                Operations.
        virtual NodePtrT const&     getNode() const                         = 0;

        virtual IStreamSetPtr_CONST getInStreams() const                    = 0;
        virtual IStreamSetPtr_CONST getOutStreams() const                   = 0;
        virtual IStreamSetPtr const&editInStreams() const                   = 0;
        virtual IStreamSetPtr const&editOutStreams() const                  = 0;
    };

public:     ////                    Operations.

    static  ThisT*                  create();

    virtual ssize_t                 setCapacity(size_t size)                = 0;

    virtual MVOID                   clear()                                 = 0;

    virtual ssize_t                 add(
                                        NodeId_T const& id,
                                        NodePtrT const& node
                                    )                                       = 0;

    virtual android::sp<INode>      getNodeFor(NodeId_T const& id) const    = 0;
    virtual android::sp<INode>      getNodeAt(size_t index) const           = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_PIPELINE_IPIPELINENODEMAPCONTROL_H_

