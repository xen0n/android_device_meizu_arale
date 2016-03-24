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

#define LOG_TAG "MtkCam/pipeline"
//
#include "MyUtils.h"
#include <mtkcam/v3/pipeline/IPipelineBufferSetFrameControl.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/**
 * An Implementation of Pipeline Frame Node Map Control.
 */
namespace {
class PipelineFrameNodeMapControlImp
    : public IPipelineFrameNodeMapControl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    struct  NodeInfo
        : public INode
    {
        NodeId_T                mNodeId;
        IStreamInfoSetPtr       mIStreams;
        IStreamInfoSetPtr       mOStreams;
        InfoIOMapSet            mIOMapSet;

                                NodeInfo(NodeId_T nodeId)
                                    : mNodeId(nodeId)
                                {}

        virtual NodeId_T        getNodeId() const                   { return mNodeId; }

        virtual IStreamInfoSetPtr_CONST
                                getIStreams() const                 { return mIStreams; }
        virtual MVOID           setIStreams(IStreamInfoSetPtr p)    { mIStreams = p; }

        virtual IStreamInfoSetPtr_CONST
                                getOStreams() const                 { return mOStreams; }
        virtual MVOID           setOStreams(IStreamInfoSetPtr p)    { mOStreams = p; }

        virtual InfoIOMapSet const&
                                getInfoIOMapSet() const             { return mIOMapSet; }
        virtual InfoIOMapSet&   editInfoIOMapSet()                  { return mIOMapSet; }
    };

    struct  NodeInfoMap
        : public android::KeyedVector<NodeId_T, android::sp<NodeInfo> >
    {
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                Data Members.
    mutable android::RWLock     mRWLock;
    NodeInfoMap                 mMap;

public:     ////                Operations.
                                PipelineFrameNodeMapControlImp();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineFrameNodeMapControl Interface..
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                Operations.
    virtual ssize_t             setCapacity(size_t size);
    virtual MVOID               clear();
    virtual ssize_t             addNode(NodeId_T const nodeId);

public:     ////                Operations.
    virtual MBOOL               isEmpty() const;
    virtual size_t              size() const;

    virtual INode*              getNodeFor(NodeId_T const nodeId) const;
    virtual INode*              getNodeAt(size_t index) const;

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
IPipelineFrameNodeMapControl*
IPipelineFrameNodeMapControl::
create()
{
    return new PipelineFrameNodeMapControlImp;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineFrameNodeMapControlImp::
PipelineFrameNodeMapControlImp()
    : mRWLock()
    , mMap()
{
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
PipelineFrameNodeMapControlImp::
setCapacity(size_t size)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    return mMap.setCapacity(size);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
PipelineFrameNodeMapControlImp::
clear()
{
    RWLock::AutoWLock _l(mRWLock);
    //
    mMap.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
PipelineFrameNodeMapControlImp::
addNode(NodeId_T const nodeId)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    return mMap.add(nodeId, new NodeInfo(nodeId));
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineFrameNodeMapControlImp::
isEmpty() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    return mMap.isEmpty();
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
PipelineFrameNodeMapControlImp::
size() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    return mMap.size();
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineFrameNodeMapControlImp::INode*
PipelineFrameNodeMapControlImp::
getNodeFor(NodeId_T const nodeId) const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    ssize_t const index = mMap.indexOfKey(nodeId);
    if  ( index < 0 ) {
        MY_LOGW("NodeId:%#" PRIxPTR " does not belong to the map", nodeId);
        for (size_t i = 0; i < mMap.size(); i++) {
            MY_LOGW("[%zu] NodeId:%#" PRIxPTR, i, mMap.keyAt(i));
        }
        return NULL;
    }
    //
    if  ( mMap.valueAt(index) == 0 ) {
        return NULL;
    }
    return mMap.valueAt(index).get();
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineFrameNodeMapControlImp::INode*
PipelineFrameNodeMapControlImp::
getNodeAt(size_t index) const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( mMap.valueAt(index) == 0 ) {
        return NULL;
    }
    return mMap.valueAt(index).get();
}

