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
#include "StreamBufferSetControlImp.h"
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;
using namespace NSCam::v3::Utils::Imp;


/******************************************************************************
 *
 ******************************************************************************/
#define MAIN_CLASS_NAME StreamBufferSetControlImp


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
sp<IUsersManager>
MAIN_CLASS_NAME::
findSubjectUsersLocked(
    StreamId_T streamId
)   const
{
#define _IMPLEMENT_(_map_) \
    { \
        ssize_t const index = _map_.indexOfKey(streamId); \
        if  ( 0 <= index ) { \
            return _map_.valueAt(index)->getUsersManager(); \
        } \
    }

    _IMPLEMENT_(mBufMap_AppImage);
    _IMPLEMENT_(mBufMap_AppMeta);
    _IMPLEMENT_(mBufMap_HalImage);
    _IMPLEMENT_(mBufMap_HalMeta);

#undef  _IMPLEMENT_

    MY_LOGW("[frame:%u] cannot find streamId:%#" PRIxPTR, mFrameNo, streamId);
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
template <class StreamBufferMapT>
sp<typename StreamBufferMapT::IStreamBufferT>
MAIN_CLASS_NAME::
getBufferLocked(
    StreamId_T streamId,
    UserId_T userId,
    StreamBufferMapT const& rBufMap
)   const
{
    if  ( 0 == rBufMap.mNumberOfNonNullBuffers ) {
        MY_LOGW_IF(
            0,
            "[frameNo:%u streamId:%#" PRIxPTR " userId:%#" PRIxPTR "] "
            "mNumberOfNonNullBuffers==0",
            getFrameNo(), streamId, userId
        );
        return NULL;
    }
    //
    typename StreamBufferMapT::value_type pValue = rBufMap.valueFor(streamId);
    if  ( pValue == 0 ) {
        MY_LOGW_IF(
            0,
            "[frameNo:%u streamId:%#" PRIxPTR " userId:%#" PRIxPTR "] "
            "cannot find from map",
            getFrameNo(), streamId, userId
        );
        return NULL;
    }
    //
    if  ( pValue->mBuffer == 0 ) {
        MY_LOGW(
            "[frameNo:%u streamId:%#" PRIxPTR " userId:%#" PRIxPTR "] "
            "mBitStatus(%#x) pValue->mBuffer == 0",
            getFrameNo(), streamId, userId, pValue->mBitStatus.value
        );
        return NULL;
    }

    /**
     * The buffer is NOT available if all users have released this buffer
     * (so as to be marked as released).
     */
    if  ( OK == pValue->getUsersManager()->haveAllUsersReleased() ) {
        MY_LOGW_IF(
            1,
            "[frameNo:%u streamId:%#" PRIxPTR " userId:%#" PRIxPTR "] "
            "all users released this buffer",
            getFrameNo(), streamId, userId
        );
        return NULL;
    }

    /**
     * For a specific stream buffer (associated with a stream Id), a user (with
     * a unique user Id) could successfully acquire the buffer from this buffer
     * set only if all users ahead of this user have pre-released or released
     * the buffer.
     */
    if  ( OK != pValue->getUsersManager()->haveAllUsersReleasedOrPreReleased(userId) ) {
        MY_LOGW_IF(
            1,
            "[frameNo:%u streamId:%#" PRIxPTR " userId:%#" PRIxPTR "] "
            "not all of prior users release or pre-release this buffer",
            getFrameNo(), streamId, userId
        );
        return NULL;
    }
    //
    //
    return pValue->mBuffer;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
MAIN_CLASS_NAME::
getMetaBufferSizeLocked() const
{
    return mBufMap_HalMeta.mNumberOfNonNullBuffers
         + mBufMap_AppMeta.mNumberOfNonNullBuffers;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
MAIN_CLASS_NAME::
getImageBufferSizeLocked()    const
{
    return mBufMap_HalImage.mNumberOfNonNullBuffers
         + mBufMap_AppImage.mNumberOfNonNullBuffers;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
MAIN_CLASS_NAME::
markUserStatus(
    StreamId_T const streamId,
    UserId_T userId,
    MUINT32 eStatus
)
{
    android::Mutex::Autolock _l(mBufMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  ( pSubjectUsers == 0 ) {
        return NAME_NOT_FOUND;
    }
    //
    return pSubjectUsers->markUserStatus(userId, eStatus);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
setUserReleaseFence(
    StreamId_T const streamId,
    UserId_T userId,
    MINT releaseFence
)
{
    android::Mutex::Autolock _l(mBufMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  ( pSubjectUsers == 0 ) {
        return NAME_NOT_FOUND;
    }
    //
    return pSubjectUsers->setUserReleaseFence(userId, releaseFence);
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
MAIN_CLASS_NAME::
queryGroupUsage(
    StreamId_T const streamId,
    UserId_T userId
)   const
{
    android::Mutex::Autolock _l(mBufMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  ( pSubjectUsers == 0 ) {
        return 0;
    }
    //
    return pSubjectUsers->queryGroupUsage(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
MAIN_CLASS_NAME::
createAcquireFence(
    StreamId_T const streamId,
    UserId_T userId
)   const
{
    android::Mutex::Autolock _l(mBufMapLock);
    //
    sp<IUsersManager> pSubjectUsers = findSubjectUsersLocked(streamId);
    if  ( pSubjectUsers == 0 ) {
        return -1;
    }
    //
    return pSubjectUsers->createAcquireFence(userId);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IStreamBufferSetControl>
IStreamBufferSetControl::
create(MUINT32 frameNo, android::wp<IAppCallback>const& pAppCallback)
{
    return new MAIN_CLASS_NAME(frameNo, pAppCallback);
}


/******************************************************************************
 *
 ******************************************************************************/
MAIN_CLASS_NAME::
MAIN_CLASS_NAME(
    MUINT32 frameNo,
    android::wp<IAppCallback>const& pAppCallback
)
    : mBufMapLock()
    , mFrameNo(frameNo)
    , mpAppCallback(pAppCallback)
{
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MAIN_CLASS_NAME::
attachListener(
    android::wp<IListener>const& pListener,
    MVOID* pCookie
)
{
    Mutex::Autolock _lBufMapLock(mBufMapLock);
    //
    mListeners.push_back(MyListener(pListener, pCookie));
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMetaStreamBuffer>
MAIN_CLASS_NAME::
getMetaBuffer(StreamId_T streamId, UserId_T userId) const
{
    sp<IMetaStreamBuffer> p;
    //
    Mutex::Autolock _lBufMapLock(mBufMapLock);
    //
    p = getBufferLocked(streamId, userId, mBufMap_HalMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getBufferLocked(streamId, userId, mBufMap_AppMeta);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IImageStreamBuffer>
MAIN_CLASS_NAME::
getImageBuffer(StreamId_T streamId, UserId_T userId) const
{
    sp<IImageStreamBuffer> p;
    //
    Mutex::Autolock _lBufMapLock(mBufMapLock);
    //
    p = getBufferLocked(streamId, userId, mBufMap_HalImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    p = getBufferLocked(streamId, userId, mBufMap_AppImage);
    if  ( p != 0 ) {
        return p;
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
struct MAIN_CLASS_NAME::TBufMapReleaser_Hal
{
public:     ////                        Data Members.
    MUINT32 const                       mFrameNo;

    android::List<android::sp<BufferMap_HalImageT::StreamBufferT> >
                                        mListToReturn_Image;
    BufferMap_HalImageT                 mrBufMap_Image;

    android::List<android::sp<BufferMap_HalMetaT::StreamBufferT> >
                                        mListToReturn_Meta;
    BufferMap_HalMetaT                  mrBufMap_Meta;

public:     ////    Operations.
    TBufMapReleaser_Hal(
        MUINT32 const frameNo,
        BufferMap_HalImageT& rBufMap_Image,
        BufferMap_HalMetaT& rBufMap_Meta
    )
        : mFrameNo(frameNo)
        //
        , mListToReturn_Image()
        , mrBufMap_Image(rBufMap_Image)
        //
        , mListToReturn_Meta()
        , mrBufMap_Meta(rBufMap_Meta)
        //
    {
    }

    MVOID
    run()
    {
        run(mFrameNo, mrBufMap_Image, mListToReturn_Image);
        run(mFrameNo, mrBufMap_Meta, mListToReturn_Meta);
    }

    template <class StreamBufferMapT, class StreamBufferListT>
    MVOID
    run(
        MUINT32 const frameNo,
        StreamBufferMapT& rBufferMap,
        StreamBufferListT& rListToReturn
    )
    {
        for (size_t i = 0; i < rBufferMap.size(); i++) {
            //
            StreamId_T const streamId = rBufferMap.keyAt(i);
            //
            typename StreamBufferMapT::value_type const&
            pValue = rBufferMap.editValueAt(i);
            if  ( pValue == 0 ) {
                MY_LOGE("[frame:%u streamId:%#" PRIxPTR "] rBufferMap.editValueAt(%zu)=NULL", frameNo, streamId, i);
                continue;
            }
            //
            //Skip if NULL buffer
            if  ( pValue->mBuffer == 0 ) {
                continue;
            }
            //
            //  [Hal Stream Buffers]
            //
            //  Condition:
            //      .This buffer is not returned before.
            //      .All users of this buffer have been released.
            //
            //  Execution:
            //      .Prepare a list of buffer to return without Release Fences.
            //
            sp<typename StreamBufferMapT::StreamBufferT>& rBuffer = pValue->mBuffer;
            BitSet32& rBitStatus = pValue->mBitStatus;
            //
            //  action if not returned && all users released
            if  (
                    ( ! rBitStatus.hasBit(eBUF_STATUS_RETURN) )
                &&  ( pValue->getUsersManager()->haveAllUsersReleased() == OK )
                )
            {
                rListToReturn.push_back(rBuffer);
                rBitStatus.markBit(eBUF_STATUS_RETURN);
                //
                rBitStatus.markBit(eBUF_STATUS_RELEASE);
                rBuffer = NULL;
                rBufferMap.mNumberOfNonNullBuffers--;
            }
        }
    }

    MVOID
    handleResult()
    {
        returnBuffers(mListToReturn_Image);
        returnBuffers(mListToReturn_Meta);
    }

    template <class T>
    MVOID
    returnBuffers(
        T& listToReturn
    )
    {
        //  Return each buffer to its pool.
        typename T::iterator it = listToReturn.begin();
        for (; it != listToReturn.end(); it++) {
            if  ( (*it) != 0 ) {
                (*it)->releaseBuffer();
            }
        }
        //
        listToReturn.clear();
    }

};


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MAIN_CLASS_NAME::
applyRelease(UserId_T userId)
{
    MY_LOGD_IF(1, "frameNo:%u userId:%#" PRIxPTR " +", mFrameNo, userId);

    sp<IAppCallback> pAppCallback;
    List<MyListener>    listeners;

    //  [Hal Image/Meta Stream Buffers]
    TBufMapReleaser_Hal releaserHal(mFrameNo, mBufMap_HalImage, mBufMap_HalMeta);
    //
    {
        Mutex::Autolock _lBufMapLock(mBufMapLock);
        //
        releaserHal.run();
        pAppCallback = mpAppCallback.promote();
        listeners = mListeners;
    }
    //
    //  Return Stream Buffers.
    releaserHal.handleResult();
    if  ( pAppCallback == 0 ) {
        MY_LOGW("Caonnot promote AppCallback for frameNo:%u , userId:%#" PRIxPTR, mFrameNo, userId);
    }
    else {
        pAppCallback->updateFrame(mFrameNo, userId);
    }
    //
    //  Notify listeners.
    {
        List<MyListener>::iterator it = listeners.begin();
        for (; it != listeners.end(); it++) {
            sp<IListener> p = it->mpListener.promote();
            if  ( p != 0 ) {
                p->onStreamBufferSet_Updated(it->mpCookie);
            }
        }
    }

    MY_LOGD_IF(1, "frameNo:%u userId:%#" PRIxPTR " -", mFrameNo, userId);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MAIN_CLASS_NAME::
applyPreRelease(UserId_T userId)
{
    applyRelease(userId);
}

