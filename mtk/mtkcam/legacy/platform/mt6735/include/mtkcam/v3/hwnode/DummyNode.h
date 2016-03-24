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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_DUMMYNODE_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_DUMMYNODE_H_
//
#include <utils/String8.h>
#include <mtkcam/v3/pipeline/IPipelineNode.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/******************************************************************************
 *
 ******************************************************************************/
//
//  [Input]
//      Meta/Request
//
//  [Output]
//      Meta/Result
//      Image/Yuv
//
class DummyNode
    : public virtual IPipelineNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    /**
     * Initialization Parameters.
     */
    struct  InitParams
    {
        /**
         * An index to indicate which camera device to open.
         */
        MINT32                      openId;

        /**
         * A unique value for the node id.
         */
        NodeId_T                    nodeId;

        /**
         * A pointer to a null-terminated string for the node name.
         */
        char const*                 nodeName;

    };

    /**
     * Configure Parameters.
     */
    struct  ConfigParams
    {
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                                    DummyNode(InitParams const& rParams);

    virtual MERROR                  config(ConfigParams const& /*params*/)
                                    {
                                        return OK;
                                    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual MINT32                  getOpenId() const   { return mOpenId; }

    virtual NodeId_T                getNodeId() const   { return mNodeId; }

    virtual char const*             getNodeName() const { return mNodeName.string(); }

    //temp
    virtual MVOID                   setCallback(android::wp<IPipelineNodeCallback> /*pCB*/) {}


    virtual MERROR                  uninit()
                                    {
                                        return OK;
                                    }

    virtual MERROR                  flush()
                                    {
                                        return OK;
                                    }
    virtual MERROR                  flush(
                                        android::sp<IPipelineFrame> const &/*pFrame */
                                    )
                                    {
                                        return OK;
                                    }

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    MINT32                          mOpenId;
    NodeId_T                        mNodeId;
    android::String8                mNodeName;

protected:  ////                    Operations.
    MVOID                           onDispatchFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );
    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V3_HWNODE_DUMMYNODE_H_

