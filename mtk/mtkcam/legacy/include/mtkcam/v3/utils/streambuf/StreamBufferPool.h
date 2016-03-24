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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_STREAMBUFFERPOOL_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_STREAMBUFFERPOOL_H_
//
#include <utils/String8.h>
#include <utils/Thread.h>
#include <utils/Vector.h>

#include "IStreamBufferPool.h"
#include "StreamBufferPoolImpl.h"

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {


/**
 * @class StreamBufferPoolImp
 *
 * @param <_IBufferT_> the type of buffer interface.
 *  This type must have operations of incStrong and decStrong.
 *
 * @param <_BufferT_> the type of buffer which is a subclass of _IBufferT_.
 *
 * @param <_AllocatorT_> the type of allocator.
 *  This type must have a call operator as below:
 *      _BufferT_* _AllocatorT_::operator()(IStreamBufferPool*);
 */
template <
    class _IBufferT_,
    class _BufferT_,
    class _AllocatorT_
>
class StreamBufferPoolImp
    : public virtual IStreamBufferPool<_IBufferT_>,
      private StreamBufferPoolImpl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    typedef android::String8                String8;

    typedef IStreamBufferPool<_IBufferT_>   IPoolT;
    typedef typename IPoolT::IBufferT       IBufferT;
    typedef typename IPoolT::SP_IBufferT    SP_IBufferT;
    typedef _BufferT_                       BufferT;
    typedef android::sp<BufferT>            SP_BufferT;
    typedef _AllocatorT_                    AllocatorT;

private:

    typedef struct
    {
        SP_BufferT  mBuf;
    } MyBufferT;
    typedef android::Vector< MyBufferT >    BufferList_t;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IStreamBufferPool Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Operations.

    virtual char const*                 poolName() const    { return mPoolName.string(); }

    virtual MVOID                       dumpPool() const;

    virtual MERROR                      initPool(
                                            char const* szCallerName,
                                            size_t maxNumberOfBuffers,
                                            size_t minNumberOfInitialCommittedBuffers
                                        );

    virtual MVOID                       uninitPool(
                                            char const* szCallerName
                                        );

    virtual MERROR                      commitPool(
                                            char const* szCallerName
                                        );

    virtual MERROR                      acquireFromPool(
                                            char const* szCallerName,
                                            SP_BufferT& rpBuffer,
                                            nsecs_t nsTimeout
                                        );

    virtual MERROR                      releaseToPool(
                                            char const* szCallerName,
                                            SP_IBufferT pBuffer
                                        );
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:
    virtual MERROR  do_construct(MUINT32& returnIndex);
            MVOID   get_itemLocation(SP_BufferT& rpBuffer, MUINT32 index);
    ////
public:     ////                        Instantiation.

    /**
     * Constructor.
     *
     * @param[in] szPoolName: a null-terminated string for a pool name.
     *
     * @param[in] rAllocator: a function object for a buffer allocator.
     *
     */
                                        StreamBufferPoolImp(
                                            char const* szPoolName,
                                            AllocatorT const& rAllocator
                                        );

    virtual                             ~StreamBufferPoolImp();

protected:  ////                        Data Members.
    mutable android::Mutex              mLock;
    String8                             mPoolName;
    AllocatorT                          mAllocator;

    BufferList_t                        mStorage;
};


/******************************************************************************
 *
 ******************************************************************************/
template <class _IBufferT_, class _BufferT_, class _AllocatorT_>
StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>::
StreamBufferPoolImp(
    char const* szPoolName,
    AllocatorT const& rAllocator
)
    : StreamBufferPoolImpl(),
      mPoolName(szPoolName),
      mAllocator(rAllocator)
{
}


template <class _IBufferT_, class _BufferT_, class _AllocatorT_>
StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>::
~StreamBufferPoolImp()
{}

/******************************************************************************
 *
 ******************************************************************************/
template <class _IBufferT_, class _BufferT_, class _AllocatorT_>
MVOID
StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>::
dumpPool() const
{
    dumpPoolImpl();
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _IBufferT_, class _BufferT_, class _AllocatorT_>
MERROR
StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>::
initPool(
    char const* szCallerName,
    size_t maxNumberOfBuffers,
    size_t minNumberOfInitialCommittedBuffers
)
{
    mStorage.setCapacity(maxNumberOfBuffers);
    //
    return initPoolImpl( szCallerName,
                         maxNumberOfBuffers,
                         minNumberOfInitialCommittedBuffers);
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _IBufferT_, class _BufferT_, class _AllocatorT_>
MVOID
StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>::
uninitPool(
    char const* szCallerName
)
{
    uninitPoolImpl( szCallerName );
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _IBufferT_, class _BufferT_, class _AllocatorT_>
MERROR
StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>::
commitPool(
    char const* szCallerName
)
{
    return commitPoolImpl( szCallerName );
    //return join();
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _IBufferT_, class _BufferT_, class _AllocatorT_>
MERROR
StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>::
acquireFromPool(
    char const*     szCallerName,
    SP_BufferT&     rpBuffer,
    nsecs_t         nsTimeout
)
{

    MUINT32 returnBufIndex = 0;
    bool  getBuf = acquireFromPoolImpl( szCallerName, returnBufIndex, nsTimeout );

    if(getBuf != OK) {
        return NO_MEMORY;
    }

    get_itemLocation(rpBuffer, returnBufIndex);
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class _IBufferT_, class _BufferT_, class _AllocatorT_>
MERROR
StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>::
releaseToPool(
    char const*     szCallerName,
    SP_IBufferT     pBuffer
)
{
    android::Mutex::Autolock _l(mLock);
    //
    MUINT32 counter = 0;
    typename android::Vector<MyBufferT>::iterator iter = mStorage.begin();
    for( ; iter != mStorage.end(); iter++ )
    {
        if( iter->mBuf.get() == pBuffer.get() ) {
            return releaseToPoolImpl( szCallerName, counter);
        }
        counter++;
    }

    return INVALID_OPERATION;
}

template <class _IBufferT_, class _BufferT_, class _AllocatorT_>
MERROR
StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>::
do_construct(MUINT32& returnIndex)
{
    SP_BufferT pBuffer = mAllocator(this);
    if( pBuffer == NULL ) {
        return NO_MEMORY;
    }

    MyBufferT buf;
    buf.mBuf = pBuffer;

    {
        android::Mutex::Autolock _l(mLock);
    mStorage.push_back(buf);
    returnIndex = mStorage.size() - 1;
    }
    return OK;
}


template <class _IBufferT_, class _BufferT_, class _AllocatorT_>
MVOID
StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>::
  get_itemLocation(SP_BufferT& rpBuffer, MUINT32 index)
{
    android::Mutex::Autolock _l(mLock);
    if( index < mStorage.size() )
        rpBuffer = mStorage[index].mBuf;
    else
        rpBuffer = NULL;
}


/**
 * @class StreamBufferPool
 *
 * @param <_IBufferT_> the type of buffer interface.
 *  This type must have operations of incStrong and decStrong.
 *
 * @param <_BufferT_> the type of buffer which is a subclass of _IBufferT_.
 *
 * @param <_AllocatorT_> the type of allocator.
 *  This type must have a call operator as below:
 *      _BufferT_* _AllocatorT_::operator()(IStreamBufferPool*);
 */
template <
    class _IBufferT_,
    class _BufferT_,
    class _AllocatorT_
>
class StreamBufferPool
    : public StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>
{
public:     ////        Definitions.
    typedef StreamBufferPoolImp<_IBufferT_, _BufferT_, _AllocatorT_>
                                            ParentT;
    typedef typename ParentT::AllocatorT    AllocatorT;
    typedef typename ParentT::IBufferT      IBufferT;
    typedef typename ParentT::BufferT       BufferT;
    typedef typename ParentT::SP_IBufferT   SP_IBufferT;
    typedef typename ParentT::SP_BufferT    SP_BufferT;

public:     ////        Operations.

    /**
     * Constructor.
     *
     * @param[in] szPoolName: a null-terminated string for a pool name.
     *
     * @param[in] rAllocator: a function object for a buffer allocator.
     *
     */
                        StreamBufferPool(
                            char const* szPoolName,
                            AllocatorT const& rAllocator
                        )
                            : ParentT(szPoolName, rAllocator)
                        {
                        }

    virtual MERROR      acquireFromPool(
                            char const* szCallerName,
                            SP_IBufferT& rpBuffer,
                            nsecs_t nsTimeout
                        )
                        {
                            SP_BufferT pBuffer;
                            MERROR err = ParentT::acquireFromPool(
                                szCallerName,
                                pBuffer,
                                nsTimeout
                            );
                            if  ( OK == err && pBuffer != 0 ) {
                                rpBuffer = pBuffer;
                            }
                            return err;
                        }


    virtual MERROR      acquireFromPool(
                            char const* szCallerName,
                            SP_BufferT& rpBuffer,
                            nsecs_t nsTimeout
                        )
                        {
                            return ParentT::acquireFromPool(
                                szCallerName,
                                rpBuffer,
                                nsTimeout
                            );
                        }
};


/**
 * @class StreamBufferPool
 *
 * @param <_BufferT_> the type of buffer.
 *  This type must have operations of incStrong and decStrong.
 *
 * @param <_AllocatorT_> the type of allocator.
 *  This type must have a call operator as below:
 *      _BufferT_* _AllocatorT_::operator()(IStreamBufferPool*);
 *
 * @remark This is a version of partial template specialization.
 */
template <
    class _BufferT_,
    class _AllocatorT_
>
class StreamBufferPool<_BufferT_, _BufferT_, _AllocatorT_>
    : public StreamBufferPoolImp<_BufferT_, _BufferT_, _AllocatorT_>
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Definitions.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////        Definitions.
    typedef StreamBufferPoolImp<_BufferT_, _BufferT_, _AllocatorT_>
                                            ParentT;
    typedef typename ParentT::AllocatorT    AllocatorT;

public:     ////        Operations.

    /**
     * Constructor.
     *
     * @param[in] szPoolName: a null-terminated string for a pool name.
     *
     * @param[in] rAllocator: a function object for a buffer allocator.
     *
     */
                        StreamBufferPool(
                            char const* szPoolName,
                            AllocatorT const& rAllocator
                        )
                            : ParentT(szPoolName, rAllocator)
                        {
                        }

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Utils
};  //namespace v3
};  //namespace NSCam

#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_STREAMBUFFERPOOL_H_

