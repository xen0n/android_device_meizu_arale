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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_STREAMBUFFERS_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_STREAMBUFFERS_H_
//
#include <mtkcam/common.h>
#include <utils/Mutex.h>
#include <utils/RWLock.h>
#include <utils/List.h>
#include <utils/String8.h>
#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>
#include <mtkcam/v3/stream/IStreamBuffer.h>
#include "UsersManager.h"
#include "StreamBufferPool.h"


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {


/******************************************************************************
 *
 ******************************************************************************/
/**
 * An implementation of stream buffer.
 */
class StreamBufferImp
    : public virtual IStreamBuffer
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.
    typedef android::status_t                   status_t;
    typedef android::Mutex                      Mutex;
    typedef android::RWLock                     RWLock;
    typedef android::String8                    String8;

protected:  ////                                Definitions.
    struct  RWUser
    {
        String8                                 name;
                                                //
                                                RWUser(char const* _name)
                                                    : name(_name)
                                                {}
    };
    typedef android::List<RWUser>               RWUserListT;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:    ////                                Data Members.
    android::sp<IStreamInfo> const              mStreamInfo;

protected:  ////                                Data Members.
    mutable android::Mutex                      mBufMutex;
    MUINT32                                     mBufStatus;
    android::sp<IUsersManager>                  mUsersManager;

protected:  ////                                Data Members.
    mutable RWUserListT                         mWriters;
    mutable RWUserListT                         mReaders;
    MVOID*const                                 mpBuffer;

protected:  ////                                Operations.
    MVOID                                       dumpLocked() const;

    virtual MVOID                               onUnlock(
                                                    char const* szCallName,
                                                    MVOID*const pBuffer
                                                );

    virtual MVOID*                              onTryReadLock(
                                                    char const* szCallName
                                                );

    virtual MVOID*                              onTryWriteLock(
                                                    char const* szCallName
                                                );

public:     ////                                Operations.
                                                StreamBufferImp(
                                                    android::sp<IStreamInfo> pStreamInfo,
                                                    MVOID*const pBuffer,
                                                    IUsersManager* pUsersManager = 0
                                                );

    virtual MVOID                               setUsersManager(
                                                    android::sp<IUsersManager> value
                                                );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IUsersManager Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Operations.
    virtual Subject_T                           getSubject() const;
    virtual char const*                         getSubjectName() const;

public:     ////                                Operations.
    virtual IUsersManager::IUserGraph*          createGraph();

    virtual ssize_t                             enqueUserGraph(
                                                    android::sp<IUserGraph> pUserGraph
                                                );

    virtual MERROR                              finishUserSetup();

    virtual MVOID                               reset();

public:     ////                                Operations.
    virtual MUINT32                             markUserStatus(
                                                    UserId_T userId,
                                                    MUINT32 const statusMask
                                                );

    virtual MUINT32                             getUserStatus(
                                                    UserId_T userId
                                                )   const;

    virtual MUINT                               getUserCategory(
                                                    UserId_T userId
                                                )   const;

    virtual MERROR                              setUserReleaseFence(
                                                    UserId_T userId,
                                                    MINT releaseFence
                                                );

    virtual MUINT                               queryGroupUsage(
                                                    UserId_T userId
                                                )   const;

    virtual size_t                              getNumberOfProducers() const;
    virtual size_t                              getNumberOfConsumers() const;

public:     ////                                Operations.
    virtual MINT                                createAcquireFence(
                                                    UserId_T userId
                                                )   const;

    virtual MINT                                createReleaseFence(
                                                    UserId_T userId
                                                )   const;

    virtual MINT                                createAcquireFence(
                                                )   const;

    virtual MINT                                createReleaseFence(
                                                )   const;

public:     ////                                Operations.
    virtual MERROR                              haveAllUsersReleasedOrPreReleased(
                                                    UserId_T userId
                                                )   const;

    virtual MERROR                              haveAllUsersReleased(
                                                )   const;

    virtual MUINT32                             getAllUsersStatus() const;

    virtual MERROR                              haveAllProducerUsersReleased(
                                                )   const;

    virtual MERROR                              haveAllProducerUsersUsed() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamBuffer Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Attributes.
    virtual char const*                         getName() const
                                                {
                                                    return mStreamInfo->getStreamName();
                                                }

    virtual StreamId_T                          getStreamId() const
                                                {
                                                    return mStreamInfo->getStreamId();
                                                }

    virtual MUINT32                             getStatus() const
                                                {
                                                    android::Mutex::Autolock _l(mBufMutex);
                                                    return mBufStatus;
                                                }

    virtual MBOOL                               hasStatus(MUINT32 mask) const
                                                {
                                                    android::Mutex::Autolock _l(mBufMutex);
                                                    return 0 != (mBufStatus & mask);
                                                }

    virtual MVOID                               markStatus(MUINT32 mask)
                                                {
                                                    android::Mutex::Autolock _l(mBufMutex);
                                                    mBufStatus |= mask;
                                                }

    virtual MVOID                               clearStatus()
                                                {
                                                    android::Mutex::Autolock _l(mBufMutex);
                                                    mBufStatus = 0;
                                                }

};


/******************************************************************************
 *
 ******************************************************************************/
/**
 * An implementation of stream buffer template.
 */
template <
    class _StreamBufferT_,
    class _IStreamBufferT_
>
class TStreamBuffer
    : public _IStreamBufferT_
    , public StreamBufferImp
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.
    typedef TStreamBuffer                       TStreamBufferT;
    typedef _StreamBufferT_                     StreamBufferT;
    typedef _IStreamBufferT_                    IStreamBufferT;
    typedef typename IStreamBufferT::IBufferT   IBufferT;
    typedef typename IStreamBufferT::IStreamInfoT
                                                IStreamInfoT;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                                Data Members.
    android::sp<IStreamInfoT> const             mStreamInfo;

public:     ////                                Operations.
                                                TStreamBuffer(
                                                    android::sp<IStreamInfoT> pStreamInfo,
                                                    IBufferT& rIBuffer,
                                                    IUsersManager* pUsersManager = 0
                                                )
                                                    : StreamBufferImp(pStreamInfo, &rIBuffer, pUsersManager)
                                                    , mStreamInfo(pStreamInfo)
                                                {
                                                }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamBuffer Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Attributes.
    virtual IStreamInfoT const*                 getStreamInfo() const
                                                {
                                                    return  mStreamInfo.get();
                                                }

public:     ////                                Operations.
    virtual MVOID                               unlock(
                                                    char const* szCallName,
                                                    IBufferT* pBuffer
                                                )
                                                {
                                                    onUnlock(szCallName, pBuffer);
                                                }

    virtual IBufferT*                           tryReadLock(
                                                    char const* szCallName
                                                )
                                                {
                                                    return (IBufferT*)onTryReadLock(szCallName);
                                                }

    virtual IBufferT*                           tryWriteLock(
                                                    char const* szCallName
                                                )
                                                {
                                                    return (IBufferT*)onTryWriteLock(szCallName);
                                                }

};


/******************************************************************************
 *
 ******************************************************************************/
/**
 * A template implementation of stream buffer with a pool.
 */
template <
    class _StreamBufferT_,
    class _IStreamBufferT_
>
class TStreamBufferWithPool
    : public TStreamBuffer<_StreamBufferT_, _IStreamBufferT_>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.
    typedef _StreamBufferT_                     StreamBufferT;
    typedef _IStreamBufferT_                    IStreamBufferT;
    typedef TStreamBuffer<StreamBufferT, IStreamBufferT>
                                                ParentT;
    typedef IStreamBufferPool<IStreamBufferT>   IStreamBufferPoolT;
    typedef typename IStreamBufferT::IBufferT   IBufferT;
    typedef typename IStreamBufferT::IStreamInfoT
                                                IStreamInfoT;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                                Data Members.
    mutable android::Mutex                      mBufPoolMutex;
    android::wp<IStreamBufferPoolT> const       mBufPool;

public:     ////                                Operations.
                                                TStreamBufferWithPool(
                                                    android::wp<IStreamBufferPoolT> pStreamBufPool,
                                                    android::sp<IStreamInfoT> pStreamInfo,
                                                    IBufferT& rIBuffer
                                                )
                                                    : ParentT(pStreamInfo, rIBuffer)
                                                    , mBufPoolMutex()
                                                    , mBufPool(pStreamBufPool)
                                                {
                                                }

public:     ////                                Operations.
    virtual android::sp<IStreamBufferPoolT>     tryGetBufferPool() const;
    virtual MVOID                               resetBuffer()               = 0;
    virtual MVOID                               releaseBuffer();

};


/******************************************************************************
 *
 ******************************************************************************/
/**
 * An implementation of hal image stream buffer.
 */
class HalImageStreamBuffer
    : public TStreamBufferWithPool<HalImageStreamBuffer, IImageStreamBuffer>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.

    struct  Allocator
    {
    public:     ////                            Definitions.
        typedef StreamBufferPool<
            IStreamBufferT,
            StreamBufferT,
            Allocator
        >                                       StreamBufferPoolT;

    public:     ////                            Data Members.
        android::sp<IStreamInfoT>               mpStreamInfo;
        IIonImageBufferHeap::AllocImgParam_t    mAllocImgParam;

    public:     ////                            Operations.
                                                Allocator(
                                                    IStreamInfoT* pStreamInfo,
                                                    IIonImageBufferHeap::AllocImgParam_t const& rAllocImgParam
                                                );

    public:     ////                            Operator.
        StreamBufferT*                          operator()(IStreamBufferPoolT* pPool = NULL);

    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                                Data Members.
    android::sp<IIonImageBufferHeap>            mImageBufferHeap;

public:     ////                                Operations.
                                                HalImageStreamBuffer(
                                                    android::sp<IStreamInfoT> pStreamInfo,
                                                    android::wp<IStreamBufferPoolT> pStreamBufPool,
                                                    android::sp<IIonImageBufferHeap>pImageBufferHeap
                                                );

public:     ////                                Operations.
    virtual MVOID                               resetBuffer();

};


/******************************************************************************
 *
 ******************************************************************************/
/**
 * An implementation of hal metadata stream buffer.
 */
class HalMetaStreamBuffer
    : public TStreamBufferWithPool<HalMetaStreamBuffer, IMetaStreamBuffer>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                Definitions.

    struct  Allocator
    {
    public:     ////                            Definitions.
        typedef StreamBufferPool<
            IStreamBufferT,
            StreamBufferT,
            Allocator
        >                                       StreamBufferPoolT;

    public:     ////                            Data Members.
        android::sp<IStreamInfoT>               mpStreamInfo;

    public:     ////                            Operations.
                                                Allocator(
                                                    IStreamInfoT* pStreamInfo
                                                );

    public:     ////                            Operator.
        StreamBufferT*                          operator()(IStreamBufferPoolT* pPool = NULL);
        StreamBufferT*                          operator()(NSCam::IMetadata const& metadata, IStreamBufferPoolT* pPool = NULL);
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                                Data Members.
    NSCam::IMetadata                            mMetadata;  // IBufferT-derived object.

public:     ////                                Operations.
                                                HalMetaStreamBuffer(
                                                    android::sp<IStreamInfoT> pStreamInfo,
                                                    android::wp<IStreamBufferPoolT> pStreamBufPool
                                                );
                                                HalMetaStreamBuffer(
                                                    NSCam::IMetadata const& metadata,
                                                    android::sp<IStreamInfoT> pStreamInfo,
                                                    android::wp<IStreamBufferPoolT> pStreamBufPool
                                                );

public:     ////                                Operations.
    virtual MVOID                               resetBuffer();

public:     ////                                Attributes.
    virtual MBOOL                               isRepeating() const { return MFALSE; }

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Utils
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_STREAMBUFFERS_H_

