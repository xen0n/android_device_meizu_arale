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
#include <utils/SortedVector.h>
#include <utils/KeyedVector.h>
#include <mtkcam/v3/pipeline/IPipelineNodeMapControl.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;


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


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class PipelineNodeMapControlImp
    : public IPipelineNodeMapControl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    struct  MyNode : public INode
    {
    protected:  ////                Data Members.
        NodePtrT                    mpNode;
        sp<IStreamInfoSetControl>   mpInStreams;
        sp<IStreamInfoSetControl>   mpOutStreams;

    public:     ////                Operations.
                                    MyNode(NodePtrT const& pNode = 0)
                                        : mpNode(pNode)
                                        , mpInStreams (IStreamInfoSetControl::create())
                                        , mpOutStreams(IStreamInfoSetControl::create())
                                    {
                                    }

    public:     ////                Operations.
        virtual NodePtrT const&     getNode() const         { return mpNode; }

    public:     ////                Operations.
        virtual IStreamSetPtr_CONST getInStreams() const    { return mpInStreams; }
        virtual IStreamSetPtr_CONST getOutStreams() const   { return mpOutStreams; }
        virtual IStreamSetPtr const&editInStreams() const   { return mpInStreams; }
        virtual IStreamSetPtr const&editOutStreams() const  { return mpOutStreams; }
    };

    typedef KeyedVector<NodeId_T, sp<MyNode> >
                                    MapT;

protected:  ////                    Data Members.
    mutable RWLock                  mRWLock;
    MapT                            mMap;

public:     ////                    Operations.
                                    PipelineNodeMapControlImp();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNodeMapControl Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual ssize_t                 setCapacity(size_t size);

    virtual MVOID                   clear();

    virtual ssize_t                 add(
                                        NodeId_T const& id,
                                        NodePtrT const& node
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNodeMap Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MBOOL                   isEmpty() const;
    virtual size_t                  size() const;

    virtual NodePtrT                nodeFor(NodeId_T const& id) const;
    virtual NodePtrT                nodeAt(size_t index) const;

    virtual sp<INode>               getNodeFor(NodeId_T const& id) const;
    virtual sp<INode>               getNodeAt(size_t index) const;

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
IPipelineNodeMapControl*
IPipelineNodeMapControl::
create()
{
    return new PipelineNodeMapControlImp;
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineNodeMapControlImp::
PipelineNodeMapControlImp()
    : mRWLock()
    , mMap()
{
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
PipelineNodeMapControlImp::
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
PipelineNodeMapControlImp::
size() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    return mMap.size();
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
PipelineNodeMapControlImp::
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
PipelineNodeMapControlImp::
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
PipelineNodeMapControlImp::
add(
    NodeId_T const& id,
    NodePtrT const& node
)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    return mMap.add(id, new MyNode(node));
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineNodeMapControlImp::NodePtrT
PipelineNodeMapControlImp::
nodeFor(NodeId_T const& id) const
{
    sp<INode> p = getNodeFor(id);
    if  ( p == 0 ) {
        MY_LOGW("Bad NodeId:%"PRIxPTR, id);
        return NULL;
    }
    //
    return p->getNode();
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineNodeMapControlImp::NodePtrT
PipelineNodeMapControlImp::
nodeAt(size_t index) const
{
    sp<INode> p = getNodeAt(index);
    if  ( p == 0 ) {
        MY_LOGW("Bad index:%zu", index);
        return NULL;
    }
    //
    return p->getNode();
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineNodeMapControl::INode>
PipelineNodeMapControlImp::
getNodeFor(NodeId_T const& id) const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    ssize_t index = mMap.indexOfKey(id);
    if  ( index < 0 ) {
        MY_LOGW("NodeId:%"PRIxPTR" does not belong to the map", id);
        for (size_t i = 0; i < mMap.size(); i++) {
            MY_LOGW("[%zu] NodeId:%"PRIxPTR, i, mMap.keyAt(i));
        }
        return NULL;
    }
    return mMap.valueAt(index);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineNodeMapControl::INode>
PipelineNodeMapControlImp::
getNodeAt(size_t index) const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    return mMap.valueAt(index);
}

