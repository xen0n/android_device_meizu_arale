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
#include <mtkcam/v3/pipeline/IPipelineFrameControl.h>
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
class PipelineFrameControlImp
    : public IPipelineFrameControl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Data Members.
    mutable RWLock                  mRWLock;
    MUINT32 const                   mFrameNo;
    MUINT32 const                   mRequestNo;
    nsecs_t                         mTimestampFrameCreated;
    nsecs_t                         mTimestampFrameDone;

protected:  ////                    Data Members.
    wp<IPipelineNodeMap const>      mpPipelineNodeMap;
    sp<IPipelineDAG const>          mpPipelineDAG;
    sp<IStreamBufferSetControl>     mpStreamBufferSet;
    sp<IStreamInfoSet const>        mpStreamInfoSet;
    KeyedVector<NodeId_T, InfoIOMapSet>
                                    mInfoIOMapSetMap;

public:     ////                    Operations.
                                    PipelineFrameControlImp(MUINT32 frameNo);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineFrameControl Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MERROR                  setPipelineNodeMap(
                                        sp<IPipelineNodeMap const> value
                                    );

    virtual MERROR                  setPipelineDAG(
                                        sp<IPipelineDAG const> value
                                    );

    virtual MERROR                  setStreamBufferSet(
                                        sp<IStreamBufferSetControl> value
                                    );

    virtual MERROR                  setStreamInfoSet(
                                        sp<IStreamInfoSet const> value
                                    );

    virtual MERROR                  addInfoIOMapSet(
                                        NodeId_T const& nodeId,
                                        InfoIOMapSet const& rIOMapSet
                                    );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineFrame Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MUINT32                 getFrameNo() const;
    virtual MUINT32                 getRequestNo() const;

    virtual android::sp<IPipelineNodeMap const>
                                    getPipelineNodeMap() const;
    virtual IPipelineDAG const&     getPipelineDAG() const;
    virtual IStreamBufferSet&       getStreamBufferSet() const;
    virtual IStreamInfoSet const&   getStreamInfoSet() const;
    virtual android::sp<IPipelineNodeCallback>
                                    getPipelineNodeCallback() const { return NULL; }
    virtual MERROR                  queryIOStreamInfoSet(
                                        NodeId_T const& /*nodeId*/,
                                        sp<IStreamInfoSet const>& /*rIn*/,
                                        sp<IStreamInfoSet const>& /*rOut*/
                                    ) const
                                    {
                                        return -EOPNOTSUPP;
                                    }

    virtual MERROR                  queryInfoIOMapSet(
                                        NodeId_T const& nodeId,
                                        InfoIOMapSet& rIOMapSet
                                    ) const;

    virtual MERROR                  attachListener(
                                        android::wp<IPipelineFrameListener>const& pListener,
                                        MVOID*const pCookie
                                    );

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
IPipelineFrameControl*
IPipelineFrameControl::
create(MUINT32 frameNo)
{
    return new PipelineFrameControlImp(frameNo);
}


/******************************************************************************
 *
 ******************************************************************************/
PipelineFrameControlImp::
PipelineFrameControlImp(MUINT32 frameNo)
    : IPipelineFrameControl()
    , mRWLock()
    , mFrameNo(frameNo)
    , mRequestNo(frameNo)
    , mTimestampFrameCreated(::elapsedRealtimeNano())
    , mTimestampFrameDone(0)
    //
    , mpPipelineNodeMap(0)
    , mpPipelineDAG(0)
    , mpStreamBufferSet(0)
    , mpStreamInfoSet(0)
    , mInfoIOMapSetMap()
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
PipelineFrameControlImp::
getFrameNo() const
{
    return mFrameNo;
}


MUINT32
PipelineFrameControlImp::
getRequestNo() const
{
    return mRequestNo;
}

/******************************************************************************
 *
 ******************************************************************************/
sp<IPipelineNodeMap const>
PipelineFrameControlImp::
getPipelineNodeMap() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    sp<IPipelineNodeMap const> p = mpPipelineNodeMap.promote();
    //
    MY_LOGE_IF(
        mpPipelineNodeMap==0 || p==0,
        "Bad PipelineNodeMap: wp:%p promote:%p - "
        "TIMESTAMP(ns) created:%" PRId64 " done:%" PRId64 " elasped:%" PRId64,
        mpPipelineNodeMap.unsafe_get(), p.get(),
        mTimestampFrameCreated, mTimestampFrameDone,
        (mTimestampFrameDone-mTimestampFrameCreated)
    );
    //
    return p;
}


/******************************************************************************
 *
 ******************************************************************************/
IPipelineDAG const&
PipelineFrameControlImp::
getPipelineDAG() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MY_LOGE_IF(
        mpPipelineDAG==0,
        "NULL PipelineDAG - "
        "TIMESTAMP(ns) created:%" PRId64 " done:%" PRId64 " elasped:%" PRId64,
        mTimestampFrameCreated, mTimestampFrameDone,
        (mTimestampFrameDone-mTimestampFrameCreated)
    );
    return *mpPipelineDAG;
}


/******************************************************************************
 *
 ******************************************************************************/
IStreamBufferSet&
PipelineFrameControlImp::
getStreamBufferSet() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MY_LOGE_IF(
        mpStreamBufferSet==0,
        "NULL StreamBufferSet - frameNo:%u "
        "TIMESTAMP(ns) created:%" PRId64 " done:%" PRId64 " elasped:%" PRId64,
        mFrameNo,
        mTimestampFrameCreated, mTimestampFrameDone,
        (mTimestampFrameDone-mTimestampFrameCreated)
    );
    return *mpStreamBufferSet;
}


/******************************************************************************
 *
 ******************************************************************************/
IStreamInfoSet const&
PipelineFrameControlImp::
getStreamInfoSet() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    MY_LOGE_IF(
        mpStreamInfoSet==0,
        "NULL StreamInfoSet - "
        "TIMESTAMP(ns) created:%" PRId64 " done:%" PRId64 " elasped:%" PRId64,
        mTimestampFrameCreated, mTimestampFrameDone,
        (mTimestampFrameDone-mTimestampFrameCreated)
    );
    return *mpStreamInfoSet;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineFrameControlImp::
queryInfoIOMapSet(
    NodeId_T const& nodeId,
    InfoIOMapSet& rIOMapSet
) const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    ssize_t const index = mInfoIOMapSetMap.indexOfKey(nodeId);
    if  ( 0 > index ) {
        MY_LOGW("nodeId:%p not found", nodeId);
        return index;
    }
    //
    rIOMapSet = mInfoIOMapSetMap.valueAt(index);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineFrameControlImp::
setPipelineNodeMap(
    android::sp<IPipelineNodeMap const> value
)
{
    if  ( value == 0) {
        MY_LOGE("NULL value");
        return BAD_VALUE;
    }
    //
    if  ( value->isEmpty() ) {
        MY_LOGE("Empty value");
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpPipelineNodeMap = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineFrameControlImp::
setPipelineDAG(android::sp<IPipelineDAG const> value)
{
    if  ( value == 0) {
        MY_LOGE("NULL value");
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpPipelineDAG = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineFrameControlImp::
setStreamInfoSet(android::sp<IStreamInfoSet const> value)
{
    if  ( value == 0) {
        MY_LOGE("NULL value");
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpStreamInfoSet = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineFrameControlImp::
addInfoIOMapSet(
    NodeId_T const& nodeId,
    InfoIOMapSet const& rIOMapSet
)
{
    RWLock::AutoWLock _l(mRWLock);
    mInfoIOMapSetMap.add(nodeId, rIOMapSet);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineFrameControlImp::
setStreamBufferSet(android::sp<IStreamBufferSetControl> value)
{
    if  ( value == 0) {
        MY_LOGE("NULL value");
        return BAD_VALUE;
    }
    //
    if  ( getFrameNo() != value->getFrameNo() ) {
        MY_LOGE(
            "Mismatch frameNo: Frame:%u StreamBufferSet:%u",
            getFrameNo(), value->getFrameNo()
        );
        return BAD_VALUE;
    }
    //
    RWLock::AutoWLock _l(mRWLock);
    mpStreamBufferSet = value;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineFrameControlImp::
attachListener(
    wp<IPipelineFrameListener>const& /*pListener*/,
    MVOID* /*pCookie*/
)
{
    return -EOPNOTSUPP;
}

