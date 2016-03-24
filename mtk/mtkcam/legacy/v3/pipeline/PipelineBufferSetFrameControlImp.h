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

#ifndef _MTK_HARDWARE_MTKCAM_V3_PIPELINE_PIPELINEBUFFERSETFRAMECONTROLIMP_H_
#define _MTK_HARDWARE_MTKCAM_V3_PIPELINE_PIPELINEBUFFERSETFRAMECONTROLIMP_H_
//
#include <mtkcam/v3/pipeline/IPipelineBufferSetFrameControl.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace NSPipelineBufferSetFrameControlImp {


/******************************************************************************
 *  Forward Declaration
 ******************************************************************************/
class ReleasedCollector;


/******************************************************************************
 *  Buffer Status
 ******************************************************************************/
enum {
    eBUF_STATUS_ACQUIRE         = 0,    //buffer has been acquired.
    eBUF_STATUS_RELEASE,                //all producers/consumers users release
    eBUF_STATUS_PRODUCERS_RELEASE,      //all producers release
};


/******************************************************************************
 *  Buffer Map
 ******************************************************************************/
struct  IMyMap
{
    struct  IItem
        : public android::RefBase
    {
        virtual IStreamInfo*    getStreamInfo() const                       = 0;
        virtual IUsersManager*  getUsersManager() const                     = 0;
        virtual android::sp<IItem>
                                handleAllUsersReleased()                    = 0;
        virtual MVOID           handleProducersReleased()                   = 0;
    };

    virtual                     ~IMyMap() {}

    virtual android::sp<IItem>  itemFor(StreamId_T streamId) const          = 0;
    virtual android::sp<IItem>  itemAt(size_t index) const                  = 0;
    virtual size_t              size() const                                = 0;
    virtual ssize_t             indexOfKey(StreamId_T const key) const      = 0;
    virtual StreamId_T          keyAt(size_t index) const                   = 0;
};


template <
    class _StreamBufferT_,
    class _IStreamBufferT_ = typename _StreamBufferT_::IStreamBufferT
>
class TItemMap
    : public IMyMap
    , public IPipelineBufferSetFrameControl::IMap<_StreamBufferT_>
{
public:     ////                Definitions.
    typedef _StreamBufferT_     StreamBufferT;
    typedef _IStreamBufferT_    IStreamBufferT;
    typedef typename StreamBufferT::IStreamInfoT
                                IStreamInfoT;

    struct  Item;
    friend  struct              Item;
    typedef Item                ItemT;
    typedef android::sp<ItemT>  MapValueT;
    typedef android::DefaultKeyedVector<StreamId_T, MapValueT>
                                MapT;

public:     ////                Data Members.
    MapT                        mMap;
    ssize_t                     mNonReleasedNum;        // In + Inout
    ReleasedCollector*          mReleasedCollector;

public:     ////                Operations.
                                TItemMap(ReleasedCollector* pReleasedCollector);
    MVOID                       onProducersReleased(ItemT& item);
    MVOID                       onAllUsersReleased(ItemT& item);

    virtual android::sp<Item>   getItemFor(StreamId_T streamId) const   { return mMap.valueFor(streamId); }

public:     ////                Operations: IMyMap
    virtual android::sp<IItem>  itemFor(StreamId_T streamId) const      { return mMap.valueFor(streamId); }
    virtual android::sp<IItem>  itemAt(size_t index) const              { return mMap.valueAt(index); }

public:     ////                Operations: IMap
    virtual ssize_t             setCapacity(size_t size)                { return mMap.setCapacity(size); }
    virtual bool                isEmpty() const                         { return mMap.isEmpty(); }
    virtual size_t              size() const                            { return mMap.size(); }
    virtual ssize_t             indexOfKey(StreamId_T const key) const  { return mMap.indexOfKey(key); }
    virtual StreamId_T          keyAt(size_t index) const               { return mMap.keyAt(index); }
    virtual IUsersManager*      usersManagerAt(size_t index) const      { return mMap.valueAt(index)->mUsersManager.get(); }
    virtual IStreamInfoT*       streamInfoAt(size_t index) const        { return mMap.valueAt(index)->mStreamInfo.get(); }

    virtual ssize_t             add(android::sp<IStreamInfoT>, android::sp<IUsersManager>);
    virtual ssize_t             add(android::sp<StreamBufferT>);
};


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
struct
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
Item : public IMyMap::IItem
{
public:     ////    Data Members.
    android::wp<TItemMap>       mItselfMap;
    android::sp<StreamBufferT>  mBuffer;
    android::sp<IStreamInfoT>   mStreamInfo;
    android::sp<IUsersManager>  mUsersManager;
    android::BitSet32           mBitStatus;

public:     ////    Operations.
                    Item(
                        android::wp<TItemMap> pItselfMap,
                        StreamBufferT*  pStreamBuffer,
                        IStreamInfoT*   pStreamInfo,
                        IUsersManager*  pUsersManager
                    )
                        : mItselfMap(pItselfMap)
                        , mBuffer(pStreamBuffer)
                        , mStreamInfo(pStreamInfo)
                        , mUsersManager(pUsersManager)
                        , mBitStatus(0)
                    {
                        if  ( pStreamBuffer != 0 ) {
                            mBitStatus.markBit(eBUF_STATUS_ACQUIRE);
                        }
                    }

    virtual IStreamInfo*    getStreamInfo() const   { return mStreamInfo.get(); }
    virtual IUsersManager*  getUsersManager() const { return mUsersManager.get(); }

    virtual android::sp<IItem>
                    handleAllUsersReleased()
                    {
                        android::sp<Item> pItem = this;
                        if  ( ! mBitStatus.hasBit(eBUF_STATUS_RELEASE) ) {
                            mBitStatus.markBit(eBUF_STATUS_RELEASE);
                            android::sp<TItemMap> pMap = mItselfMap.promote();
                            if  ( pMap != 0 ) {
                                pMap->onAllUsersReleased(*this);
                            }
                        }
                        return pItem;
                    }

    virtual MVOID   handleProducersReleased()
                    {
                        if  ( ! mBitStatus.hasBit(eBUF_STATUS_PRODUCERS_RELEASE) ) {
                            mBitStatus.markBit(eBUF_STATUS_PRODUCERS_RELEASE);
                            android::sp<TItemMap> pMap = mItselfMap.promote();
                            if  ( pMap != 0 ) {
                                pMap->onProducersReleased(*this);
                            }
                        }
                    }
};


typedef TItemMap<IImageStreamBuffer, IImageStreamBuffer>                ItemMap_AppImageT;
typedef TItemMap< IMetaStreamBuffer,  IMetaStreamBuffer>                ItemMap_AppMetaT;
typedef TItemMap<IPipelineBufferSetFrameControl::HalImageStreamBuffer>  ItemMap_HalImageT;
typedef TItemMap<IPipelineBufferSetFrameControl:: HalMetaStreamBuffer>  ItemMap_HalMetaT;


/******************************************************************************
 *  Frame Releaser
 ******************************************************************************/
class ReleasedCollector
    : public android::RefBase
{
public:     ////    Definitions.
    typedef ItemMap_HalImageT::StreamBufferT    HalImageStreamBufferT;
    typedef ItemMap_HalMetaT ::StreamBufferT    HalMetaStreamBufferT;
    typedef ItemMap_AppImageT::StreamBufferT    AppImageStreamBufferT;
    typedef ItemMap_AppMetaT ::StreamBufferT    AppMetaStreamBufferT;

    typedef android::Vector<android::sp<HalImageStreamBufferT> >
                                                HalImageSetT;
    typedef android::Vector<android::sp<HalMetaStreamBufferT> >
                                                HalMetaSetT;
    typedef android::Vector<android::sp<AppMetaStreamBufferT> >
                                                AppMetaSetT;

public:     ////    Data Members.
    mutable android::Mutex
                    mLock;
    HalImageSetT    mHalImageSet_AllUsersReleased;
    HalMetaSetT     mHalMetaSet_AllUsersReleased;

    AppMetaSetT     mAppMetaSetO_ProducersReleased; //out
    ssize_t         mAppMetaNumO_ProducersInFlight; //out

public:     ////    Operations.
    MVOID           finishConfiguration(
                        ItemMap_AppImageT const&,
                        ItemMap_AppMetaT const& rItemMap_AppMeta
                    )
                    {
                        {
                            mAppMetaNumO_ProducersInFlight = 0;
                            ItemMap_AppMetaT const& rItemMap = rItemMap_AppMeta;
                            for (size_t i = 0; i < rItemMap.size(); i++) {
                                if  ( 0 < rItemMap.usersManagerAt(i)->getNumberOfProducers() ) {
                                    mAppMetaNumO_ProducersInFlight++;
                                }
                            }
                        }
                    }

public:     ////    Operations.
    MVOID           onAllUsersReleased(ItemMap_AppImageT::Item&) {}
    MVOID           onAllUsersReleased(ItemMap_AppMetaT::Item&) {}
    MVOID           onAllUsersReleased(ItemMap_HalImageT::Item& rItem)
                    {
                        android::Mutex::Autolock _l(mLock);
                        if  ( rItem.mBuffer != 0 ) {
                            mHalImageSet_AllUsersReleased.push_back(rItem.mBuffer);
                        }
                    }
    MVOID           onAllUsersReleased(ItemMap_HalMetaT::Item& rItem)
                    {
                        android::Mutex::Autolock _l(mLock);
                        if  ( rItem.mBuffer != 0 ) {
                            mHalMetaSet_AllUsersReleased.push_back(rItem.mBuffer);
                        }
                    }

public:     ////    Operations.
    MVOID           onProducersReleased(ItemMap_HalImageT::Item&) {}
    MVOID           onProducersReleased(ItemMap_HalMetaT::Item&) {}
    MVOID           onProducersReleased(ItemMap_AppImageT::Item&) {}
    MVOID           onProducersReleased(ItemMap_AppMetaT::Item& rItem)
                    {
                        android::Mutex::Autolock _l(mLock);
                        if  ( 0 < rItem.getUsersManager()->getNumberOfProducers() ) {
                            mAppMetaNumO_ProducersInFlight--;
                            if  ( rItem.mBuffer != 0 ) {
                                mAppMetaSetO_ProducersReleased.push_back(rItem.mBuffer);
                            }
                        }
                    }
};


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
TItemMap(ReleasedCollector* pReleasedCollector)
    : mMap()
    , mNonReleasedNum(0)
    , mReleasedCollector(pReleasedCollector)
{}


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
MVOID
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
onProducersReleased(ItemT& item)
{
    if  ( mReleasedCollector != 0 ) {
        mReleasedCollector->onProducersReleased(item);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
MVOID
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
onAllUsersReleased(ItemT& item)
{
    android::sp<ItemT>& rpItem = mMap.editValueFor(item.mStreamInfo->getStreamId());
    if  ( rpItem != 0 ) {
        if  ( mReleasedCollector != 0 ) {
            mReleasedCollector->onAllUsersReleased(item);
        }
        mNonReleasedNum--;
        rpItem = NULL;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
ssize_t
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
add(
    android::sp<IStreamInfoT> pStreamInfo,
    android::sp<IUsersManager> pUsersManager
)
{
    StreamId_T const streamId = pStreamInfo->getStreamId();
    //
    if  ( pUsersManager == 0 ) {
        pUsersManager = new NSCam::v3::Utils::UsersManager(
            streamId, pStreamInfo->getStreamName()
        );
    }
    //
    mNonReleasedNum++;
    return mMap.add(
        streamId,
        new ItemT(this, 0, pStreamInfo.get(), pUsersManager.get())
    );
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
ssize_t
TItemMap<_StreamBufferT_, _IStreamBufferT_>::
add(android::sp<StreamBufferT> value)
{
    if  ( value == 0 ) {
        return BAD_VALUE;
    }
    //
    StreamId_T const streamId = value->getStreamInfo()->getStreamId();
    //
    mNonReleasedNum++;
    return mMap.add(
        streamId,
        new ItemT(this, value.get(), const_cast<IStreamInfoT*>(value->getStreamInfo()), value.get())
    );
}


/**
 * An Implementation of Pipeline Buffer Set Frame Control.
 */
class PipelineBufferSetFrameControlImp
    : public IPipelineBufferSetFrameControl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Frame Listener
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    struct  MyListener
    {
        typedef IPipelineFrameListener
                                    IListener;
        android::wp<IListener>      mpListener;
        MVOID*                      mpCookie;
                                    //
                                    MyListener(
                                        android::wp<IListener> listener = NULL,
                                        MVOID*const cookie = NULL
                                    )
                                        : mpListener(listener)
                                        , mpCookie(cookie)
                                    {}
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions: Node Status
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    struct  NodeStatus
        : public android::LightRefBase<NodeStatus>
    {
        struct IO : public android::LightRefBase<IO>
        {
            android::sp<IMyMap::IItem>
                                    mMapItem;
        };

        struct  IOSet
            : public android::List<android::sp<IO> >
        {
            MBOOL                   mNotified;
                                    //
                                    IOSet()
                                        : mNotified(MFALSE)
                                    {}
        };

        IOSet                       mISetImage;
        IOSet                       mOSetImage;
        IOSet                       mISetMeta;
        IOSet                       mOSetMeta;
    };

    struct  NodeStatusMap
        : public android::KeyedVector<NodeId_T, android::sp<NodeStatus> >
    {
        size_t                      mInFlightNodeCount;
                                    //
                                    NodeStatusMap()
                                        : mInFlightNodeCount(0)
                                    {}
    };

    struct  NodeStatusUpdater;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data Members
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                    Common.
    MUINT32 const                   mFrameNo;
    MUINT32 const                   mRequestNo;
    mutable android::RWLock         mRWLock;
    android::wp<IAppCallback>const  mpAppCallback;
    android::List<MyListener>       mListeners;
    nsecs_t                         mTimestampFrameCreated;
    nsecs_t                         mTimestampFrameDone;
    MINT32                          mLogLevel;

protected:  ////                    Configuration.
    android::wp<IPipelineStreamBufferProvider>
                                    mBufferProvider;

    android::wp<IPipelineNodeCallback>
                                    mpPipelineCallback;

    android::sp<IStreamInfoSet const>
                                    mpStreamInfoSet;

    android::sp<IPipelineFrameNodeMapControl>
                                    mpNodeMap;

    android::wp<IPipelineNodeMap const>
                                    mpPipelineNodeMap;

    android::sp<IPipelineDAG const> mpPipelineDAG;

protected:  ////
    mutable android::Mutex          mItemMapLock;
    NodeStatusMap                   mNodeStatusMap;
    android::sp<ReleasedCollector>  mpReleasedCollector;
    android::sp<ItemMap_AppImageT>  mpItemMap_AppImage;
    android::sp<ItemMap_AppMetaT>   mpItemMap_AppMeta;
    android::sp<ItemMap_HalImageT>  mpItemMap_HalImage;
    android::sp<ItemMap_HalMetaT>   mpItemMap_HalMeta;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                                    PipelineBufferSetFrameControlImp(
                                        MUINT32 requestNo,
                                        MUINT32 frameNo,
                                        android::wp<IAppCallback>const& pAppCallback,
                                        IPipelineStreamBufferProvider const* pBufferProvider,
                                        android::wp<IPipelineNodeCallback> pNodeCallback
                                    );

protected:  ////                    Operations.
    MVOID                           handleReleasedBuffers(
                                        UserId_T userId,
                                        android::sp<IAppCallback> pAppCallback
                                    );

protected:  ////                    Operations.
    android::sp<IUsersManager>      findSubjectUsersLocked(
                                        StreamId_T streamId
                                    )   const;

    android::sp<IMyMap::IItem>      getMapItemLocked(
                                        StreamId_T streamId,
                                        IMyMap const& rItemMap
                                    )   const;

    android::sp<IMyMap::IItem>      getMetaMapItemLocked(StreamId_T streamId) const;
    android::sp<IMyMap::IItem>      getImageMapItemLocked(StreamId_T streamId) const;

    template <class ItemMapT>
    android::sp<typename ItemMapT::IStreamBufferT>
                                    getBufferLocked(
                                        StreamId_T streamId,
                                        ItemMapT const& rMap
                                    )   const;

    template <class ItemMapT>
    android::sp<typename ItemMapT::IStreamBufferT>
                                    getBufferLocked(
                                        StreamId_T streamId,
                                        UserId_T userId,
                                        ItemMapT const& rMap
                                    )   const;

    virtual android::sp<IMetaStreamBuffer>
                                    getMetaStreamBuffer(
                                        StreamId_T streamId
                                    )   const;

    virtual android::sp<IImageStreamBuffer>
                                    getImageStreamBuffer(
                                        StreamId_T streamId
                                    )   const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetFrameControl Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Configuration.
    virtual MERROR                  startConfiguration();
    virtual MERROR                  finishConfiguration();

    virtual MERROR                  setNodeMap(android::sp<IPipelineFrameNodeMapControl> value);
    virtual MERROR                  setPipelineNodeMap(android::sp<IPipelineNodeMap const> value);
    virtual MERROR                  setPipelineDAG(android::sp<IPipelineDAG const> value);
    virtual MERROR                  setStreamInfoSet(android::sp<IStreamInfoSet const> value);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineBufferSetControl Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations

#define _EDITMAP_(_NAME_, _TYPE_) \
    virtual android::sp<IMap< ItemMap_##_NAME_##_TYPE_##T::StreamBufferT > > \
            editMap_##_NAME_##_TYPE_() \
            { \
                return mpItemMap_##_NAME_##_TYPE_; \
            }

    _EDITMAP_(Hal, Image)   // editMap_HalImage
    _EDITMAP_(App, Image)   // editMap_AppImage
    _EDITMAP_(Hal, Meta)    // editMap_HalMeta
    _EDITMAP_(App, Meta)    // editMap_AppMeta

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamBufferSet Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
    virtual MVOID                   applyPreRelease(UserId_T userId){ applyRelease(userId); }
    virtual MVOID                   applyRelease(UserId_T userId);

    virtual MUINT32                 markUserStatus(
                                        StreamId_T const streamId,
                                        UserId_T userId,
                                        MUINT32 eStatus
                                    );

    virtual MERROR                  setUserReleaseFence(
                                        StreamId_T const streamId,
                                        UserId_T userId,
                                        MINT releaseFence
                                    );

    virtual MUINT                   queryGroupUsage(
                                        StreamId_T const streamId,
                                        UserId_T userId
                                    )   const;

    virtual MINT                    createAcquireFence(
                                        StreamId_T const streamId,
                                        UserId_T userId
                                    )   const;

public:     ////                    Operations.
    virtual android::sp<IMetaStreamBuffer>
                                    getMetaBuffer(
                                        StreamId_T streamId,
                                        UserId_T userId
                                    )   const;

    virtual android::sp<IImageStreamBuffer>
                                    getImageBuffer(
                                        StreamId_T streamId,
                                        UserId_T userId
                                    )   const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineFrame Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Attributes.
    virtual MUINT32                 getFrameNo() const   { return mFrameNo; }
    virtual MUINT32                 getRequestNo() const { return mRequestNo; }

public:     ////                    Operations.
    virtual MERROR                  attachListener(
                                        android::wp<IPipelineFrameListener>const&,
                                        MVOID*const pCookie
                                    );

    virtual android::sp<IPipelineNodeMap const>
                                    getPipelineNodeMap() const;
    virtual IPipelineDAG const&     getPipelineDAG() const;
    virtual IStreamInfoSet const&   getStreamInfoSet() const;
    virtual IStreamBufferSet&       getStreamBufferSet() const;
    virtual android::sp<IPipelineNodeCallback>
                                    getPipelineNodeCallback() const;

    virtual MERROR                  queryIOStreamInfoSet(
                                        NodeId_T const& nodeId,
                                        android::sp<IStreamInfoSet const>& rIn,
                                        android::sp<IStreamInfoSet const>& rOut
                                    )   const;

    virtual MERROR                  queryInfoIOMapSet(
                                        NodeId_T const& nodeId,
                                        InfoIOMapSet& rIOMapSet
                                    )   const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  RefBase Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    virtual void                    onLastStrongRef(const void* id);
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPipelineBufferSetFrameControlImp
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_MTKCAM_V3_PIPELINE_PIPELINEBUFFERSETFRAMECONTROLIMP_H_

