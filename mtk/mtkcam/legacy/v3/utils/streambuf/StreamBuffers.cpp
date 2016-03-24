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

#define LOG_TAG "MtkCam/streambuf"
//
#include "MyUtils.h"
#include <mtkcam/v3/utils/streambuf/StreamBuffers.h>
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
StreamBufferImp::
StreamBufferImp(
    android::sp<IStreamInfo> pStreamInfo,
    MVOID*const pBuffer,
    IUsersManager* pUsersManager
)
    : mStreamInfo(pStreamInfo)
    , mBufMutex()
    , mBufStatus(0)
    , mUsersManager(pUsersManager!=0 ? pUsersManager : new UsersManager(pStreamInfo->getStreamId(), pStreamInfo->getStreamName()))
    //
    , mWriters()
    , mReaders()
    , mpBuffer(pBuffer)
{
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamBufferImp::
setUsersManager(
    android::sp<IUsersManager> value
)
{
    mUsersManager = value;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamBufferImp::
dumpLocked() const
{
    for (
            typename RWUserListT::iterator it = mWriters.begin()
        ;   it != mWriters.end()
        ;   it++
        )
    {
        MY_LOGD(" [%s] Writer %s", getName(), it->name.string());
    }
    //
    for (
            typename RWUserListT::iterator it = mReaders.begin()
        ;   it != mReaders.end()
        ;   it++
        )
    {
        MY_LOGD(" [%s] Reader %s", getName(), it->name.string());
    }
    //
}



/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamBufferImp::
onUnlock(
    char const* szCallName,
    MVOID*const pBuffer
)
{
    Mutex::Autolock _l(mBufMutex);
    //
    if  ( mpBuffer != pBuffer ) {
        MY_LOGE("[%s:%p] %s cannot unlock buffer:%p", getName(), mpBuffer, szCallName, pBuffer);
        dumpLocked();
        return;
    }
    //
    for (
            typename RWUserListT::iterator it = mWriters.begin()
        ;   it != mWriters.end()
        ;   it++
        )
    {
        if  ( it->name == szCallName ) {
            mWriters.erase(it);
            MY_LOGV("[%s:%p] Writer %s", getName(), mpBuffer, szCallName);
            return;
        }
    }
    //
    for (
            typename RWUserListT::iterator it = mReaders.begin()
        ;   it != mReaders.end()
        ;   it++
        )
    {
        if  ( it->name == szCallName ) {
            mReaders.erase(it);
            MY_LOGV("[%s:%p] Reader %s", getName(), mpBuffer, szCallName);
            return;
        }
    }
    //
    MY_LOGE("[%s:%p] %s has not locked buffer", getName(), mpBuffer, szCallName);
    dumpLocked();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID*
StreamBufferImp::
onTryReadLock(
    char const* szCallName
)
{
    Mutex::Autolock _l(mBufMutex);
    //
    if  ( ! mWriters.empty() ) {
        MY_LOGV("[%s:%p] Writers %s:exist; %s:denied", getName(), mpBuffer, mWriters.begin()->name.string(), szCallName);
        return NULL;
    }
    //
    MY_LOGV("[%s:%p] Reader %s", getName(), mpBuffer, szCallName);
    mReaders.push_back(RWUser(szCallName));
    return  mpBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID*
StreamBufferImp::
onTryWriteLock(
    char const* szCallName
)
{
    Mutex::Autolock _l(mBufMutex);
    //
    if  ( ! mWriters.empty() ) {
        MY_LOGV("[%s:%p] Writers %s:exist; %s:denied", getName(), mpBuffer, mWriters.begin()->name.string(), szCallName);
        return NULL;
    }
    if  ( ! mReaders.empty() ) {
        MY_LOGV("[%s:%p] Readers %s:exist; %s:denied", getName(), mpBuffer, mReaders.begin()->name.string(), szCallName);
        return NULL;
    }
    //
    MY_LOGV("[%s:%p] Writer %s", getName(), mpBuffer, szCallName);
    mWriters.push_back(RWUser(szCallName));
    return  mpBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
IUsersManager::Subject_T
StreamBufferImp::
getSubject() const
{
    return mUsersManager->getSubject();
}


/******************************************************************************
 *
 ******************************************************************************/
char const*
StreamBufferImp::
getSubjectName() const
{
    return mUsersManager->getSubjectName();
}


/******************************************************************************
 *
 ******************************************************************************/
IUsersManager::IUserGraph*
StreamBufferImp::
createGraph()
{
    return mUsersManager->createGraph();
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
StreamBufferImp::
enqueUserGraph(
    android::sp<IUserGraph> pUserGraph
)
{
    return mUsersManager->enqueUserGraph(pUserGraph);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferImp::
finishUserSetup()
{
    return mUsersManager->finishUserSetup();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamBufferImp::
reset()
{
    mUsersManager->reset();
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
StreamBufferImp::
markUserStatus(
    UserId_T userId,
    MUINT32 const statusMask
)
{
    Mutex::Autolock _l(mBufMutex);  //This call is to avoid race condition,
                                    //since buffer status and users status are
                                    //in the same critical section.
    //
    MUINT32 const result = mUsersManager->markUserStatus(userId, statusMask);
    //
    bool const isReleased   = result & (UserStatus::RELEASE);
    bool const isUsed       = result & (UserStatus::USED);
    if  (
          ! isUsed
        &&  isReleased
        &&  Category::PRODUCER == mUsersManager->getUserCategory(userId)
        )
    {
        //  This producer user has released the buffer without producing its
        //  content, its content must be ERROR.  It may happen when a producer
        //  tries to flush or cancel the buffer.
        MY_LOGW(
            "%s:%#" PRIxPTR ":%p producer:%#" PRIxPTR " released w/o using -> ERROR buffer - user status:%#x:%#x",
            getName(), getStreamId(), mpBuffer, userId, statusMask, result
        );
        mBufStatus |= STREAM_BUFFER_STATUS::ERROR;
    }
    //
    return result;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
StreamBufferImp::
getUserStatus(
    UserId_T userId
)   const
{
    return mUsersManager->getUserStatus(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
StreamBufferImp::
getUserCategory(
    UserId_T userId
)   const
{
    return mUsersManager->getUserCategory(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferImp::
setUserReleaseFence(
    UserId_T userId,
    MINT releaseFence
)
{
    return mUsersManager->setUserReleaseFence(userId, releaseFence);
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
StreamBufferImp::
queryGroupUsage(
    UserId_T userId
)   const
{
    return mUsersManager->queryGroupUsage(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
StreamBufferImp::
getNumberOfProducers() const
{
    return mUsersManager->getNumberOfProducers();
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
StreamBufferImp::
getNumberOfConsumers() const
{
    return mUsersManager->getNumberOfConsumers();
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
StreamBufferImp::
createAcquireFence(
    UserId_T userId
)   const
{
    return mUsersManager->createAcquireFence(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
StreamBufferImp::
createReleaseFence(
    UserId_T userId
)   const
{
    return mUsersManager->createReleaseFence(userId);
}

/******************************************************************************
 *
 ******************************************************************************/
MINT
StreamBufferImp::
createAcquireFence(
)   const
{
    return mUsersManager->createAcquireFence();
}

/******************************************************************************
 *
 ******************************************************************************/
MINT
StreamBufferImp::
createReleaseFence(
)   const
{
    return mUsersManager->createReleaseFence();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferImp::
haveAllUsersReleasedOrPreReleased(
    UserId_T userId
)   const
{
    return mUsersManager->haveAllUsersReleasedOrPreReleased(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferImp::
haveAllUsersReleased(
)   const
{
    return mUsersManager->haveAllUsersReleased();
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
StreamBufferImp::
getAllUsersStatus() const
{
    return mUsersManager->getAllUsersStatus();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferImp::
haveAllProducerUsersReleased() const
{
    return mUsersManager->haveAllProducerUsersReleased();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferImp::
haveAllProducerUsersUsed() const
{
    return mUsersManager->haveAllProducerUsersUsed();
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
android::sp<IStreamBufferPool<_IStreamBufferT_> >
TStreamBufferWithPool<_StreamBufferT_, _IStreamBufferT_>::
tryGetBufferPool() const
{
    Mutex::Autolock _l(mBufPoolMutex);
    //
    if  ( mBufPool == 0 ) {
        MY_LOGV("[%s:%p] no buffer pool", ParentT::getName(), this);
        return NULL;
    }
    //
    sp<IStreamBufferPoolT> const pPool = mBufPool.promote();
    if  ( pPool == 0 ) {
        MY_LOGW(
            "[%s:%p] NULL promote of buffer pool:%p",
            ParentT::getName(), this, mBufPool.unsafe_get()
        );
    }
    //
    return pPool;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _StreamBufferT_, class _IStreamBufferT_>
MVOID
TStreamBufferWithPool<_StreamBufferT_, _IStreamBufferT_>::
releaseBuffer()
{
    MY_LOGD_IF(0, "+ %s", ParentT::getName());
    sp<IStreamBufferPoolT> pStreamBufferPool = tryGetBufferPool();
    if  ( pStreamBufferPool != 0 ) {
        //
        //  Reset buffer before returning to pool.
        resetBuffer();
        //
        //  Release to Pool
        MERROR err = pStreamBufferPool->releaseToPool(
            ParentT::getName(),
            this
        );
        MY_LOGE_IF(OK!=err, "%s fail to release to pool", ParentT::getName());
    }
    MY_LOGD_IF(0, "- %s", ParentT::getName());
}


/******************************************************************************
 *
 ******************************************************************************/
HalImageStreamBuffer::
Allocator::
Allocator(
    IStreamInfoT* pStreamInfo,
    IIonImageBufferHeap::AllocImgParam_t const& rAllocImgParam
)
    : mpStreamInfo(pStreamInfo)
    , mAllocImgParam(rAllocImgParam)
{
}


/******************************************************************************
 *
 ******************************************************************************/
HalImageStreamBuffer::StreamBufferT*
HalImageStreamBuffer::
Allocator::
operator()(IStreamBufferPoolT* pPool)
{
    sp<IIonImageBufferHeap>
    pImageBufferHeap =
    IIonImageBufferHeap::create(
        mpStreamInfo->getStreamName(),
        mAllocImgParam,
        IIonImageBufferHeap::AllocExtraParam(),
        MFALSE
    );
    if  ( pImageBufferHeap == 0 ) {
        MY_LOGE(
            "IIonImageBufferHeap::create: %s",
            mpStreamInfo->getStreamName()
        );
        return NULL;
    }
    //
    return new StreamBufferT(
        mpStreamInfo,
        pPool,
        pImageBufferHeap
    );
}


/******************************************************************************
 *
 ******************************************************************************/
HalImageStreamBuffer::
HalImageStreamBuffer(
    sp<IStreamInfoT> pStreamInfo,
    wp<IStreamBufferPoolT> pStreamBufPool,
    sp<IIonImageBufferHeap>pImageBufferHeap
)
    : TStreamBufferWithPool(pStreamBufPool, pStreamInfo, *pImageBufferHeap.get())
    , mImageBufferHeap(pImageBufferHeap)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalImageStreamBuffer::
resetBuffer()
{
    reset();
    //
    {
        Mutex::Autolock _l(mBufMutex);
        mBufStatus = 0;
        mWriters.clear();
        mReaders.clear();
    }
}


/******************************************************************************
 *
 ******************************************************************************/
HalMetaStreamBuffer::
Allocator::
Allocator(
    IStreamInfoT* pStreamInfo
)
    : mpStreamInfo(pStreamInfo)
{
}


/******************************************************************************
 *
 ******************************************************************************/
HalMetaStreamBuffer::StreamBufferT*
HalMetaStreamBuffer::
Allocator::
operator()(IStreamBufferPoolT* pPool)
{
    return new StreamBufferT(
        mpStreamInfo,
        pPool
    );
}


/******************************************************************************
 *
 ******************************************************************************/
HalMetaStreamBuffer::StreamBufferT*
HalMetaStreamBuffer::
Allocator::
operator()(NSCam::IMetadata const& metadata, IStreamBufferPoolT* pPool)
{
    return new StreamBufferT(
        metadata,
        mpStreamInfo,
        pPool
    );
}


/******************************************************************************
 *
 ******************************************************************************/
HalMetaStreamBuffer::
HalMetaStreamBuffer(
    sp<IStreamInfoT> pStreamInfo,
    wp<IStreamBufferPoolT> pStreamBufPool
)
    : TStreamBufferWithPool(pStreamBufPool, pStreamInfo, mMetadata)
    , mMetadata()
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
HalMetaStreamBuffer::
HalMetaStreamBuffer(
    NSCam::IMetadata const& metadata,
    sp<IStreamInfoT> pStreamInfo,
    wp<IStreamBufferPoolT> pStreamBufPool
)
    : TStreamBufferWithPool(pStreamBufPool, pStreamInfo, mMetadata)
    , mMetadata(metadata)
    //
{
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalMetaStreamBuffer::
resetBuffer()
{
    reset();
    //
    {
        Mutex::Autolock _l(mBufMutex);
        mBufStatus = 0;
        mWriters.clear();
        mReaders.clear();
        mMetadata.clear();
    }
}

