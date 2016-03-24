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
#include <mtkcam/v3/utils/streambuf/UsersManager.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;
using namespace NSCam::Utils::Sync;


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
UsersManager::
MyUser::
MyUser(User const& user, ssize_t groupIndex)
    : mUserId(user.mUserId)
    , mAcquireFence(IFence::create(user.mAcquireFence))
    , mReleaseFence(IFence::create(user.mReleaseFence))
    , mUsage(user.mUsage)
    , mCategory(user.mCategory)
    //
    , mGroupIndex(groupIndex)
    , mUserStatus(0)
{
}


/******************************************************************************
 *
 ******************************************************************************/
UsersManager::
MyUserGraph::
MyUserGraph(size_t groupIndex)
    : mRWLock()
    , mConsumerUsage(0)
    , mGroupIndex(groupIndex)
{
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
MyUserGraph::
addUser(User const& usr)
{
    RWLock::AutoWLock _(mRWLock);
    //
    ssize_t idx = mUserVector.indexOfKey(usr.mUserId);
    if (0 <= idx)
    {
        MY_LOGE("UserId %zd already exists", idx);
        return ALREADY_EXISTS;
    }
    //
    ssize_t err = mUserVector.add(usr.mUserId, UserNode(new MyUser(usr, mGroupIndex)));
    if  (0 > err) {
        MY_LOGE("Not enough memory");
        return err;
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
MyUserGraph::
removeUser(UserId_T id)
{
    RWLock::AutoWLock _(mRWLock);
    //
    ssize_t rmIdx = mUserVector.indexOfKey(id);
    if (0 > rmIdx)
    {
        //MY_LOGE("The user of id %" PRIxPTR " does not exist",id);
        return rmIdx;
    }
    android::SortedVector<UserId_T> *rmSet;
    //remove in adjacent nodes of deleted node
    rmSet = &(mUserVector.editValueAt(rmIdx).mIn);
    size_t upper = rmSet->size();
    for (size_t i = 0; i < upper; i++)
    {
        mUserVector.editValueFor(rmSet->itemAt(i)).mOut.remove(id);
    }
    //remove out adjacent nodes of deleted node
    rmSet = &(mUserVector.editValueAt(rmIdx).mOut);
    upper = rmSet->size();
    for (size_t i = 0; i < upper; i++)
    {
        mUserVector.editValueFor(rmSet->itemAt(i)).mIn.remove(id);
    }
    mUserVector.removeItemsAt(rmIdx);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
MyUserGraph::
addEdge(UserId_T id_src, UserId_T id_dst)
{
    RWLock::AutoWLock _(mRWLock);
    //Ensure both nodes exist
    ssize_t srcIdx = mUserVector.indexOfKey(id_src);
    ssize_t dstIdx = mUserVector.indexOfKey(id_dst);
    if (0 > srcIdx || 0 > dstIdx)
    {
        //MY_LOGE("User does not exist\nSrc ID:%" PRIxPTR " Src Index:%zd Dst ID:%" PRIxPTR " Dst Index:%zd\n", id_src, srcIdx, id_dst, dstIdx);
        return NAME_NOT_FOUND;
    }

    //Ensure that each edge only be added once
    ssize_t srcEdgeIdx = mUserVector.valueAt(srcIdx).mOut.indexOf(id_dst);
    ssize_t dstEdgeIdx = mUserVector.valueAt(dstIdx).mIn.indexOf(id_src);
    if (0 > srcEdgeIdx && 0 > dstEdgeIdx)
    {
        ssize_t err_src = mUserVector.editValueAt(srcIdx).mOut.add(id_dst);
        ssize_t err_dst = mUserVector.editValueAt(dstIdx).mIn.add(id_src);
        if (0 > err_src || 0 > err_dst)
        {
            MY_LOGE("Not enough memory\nSrc ID:%" PRIxPTR " Src Index:%zd Dst ID:%" PRIxPTR " Dst Index:%zd\n", id_src, err_src, id_dst, err_dst);
            return NO_MEMORY;
        }
    }
    else if (0 <= srcEdgeIdx && 0 <= dstEdgeIdx)
    {
        MY_LOGE("Edge already exists: srcIdx(%zd) <--> dstIdx(%zd)", srcIdx, dstIdx);
        return ALREADY_EXISTS;
    }
    else if (0 <= srcEdgeIdx)
    {
        MY_LOGE("Illegal edge exists: srcIdx(%zd) <-X- dstIdx(%zd)", srcIdx, dstIdx);
        return ALREADY_EXISTS;
    }
    else
    {
        MY_LOGE("Illegal edge exists: srcIdx(%zd) -X-> dstIdx(%zd)", srcIdx, dstIdx);
        return ALREADY_EXISTS;
    }


    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
MyUserGraph::
removeEdge(UserId_T id_src, UserId_T id_dst)
{
    RWLock::AutoWLock _(mRWLock);
    ssize_t srcIdx = mUserVector.indexOfKey(id_src);
    ssize_t dstIdx = mUserVector.indexOfKey(id_dst);
    if (0 > srcIdx || 0 > dstIdx)
    {
        //MY_LOGE("User does not exist\nSrc ID:%" PRIxPTR " Src Index:%zd Dst ID:%" PRIxPTR " Dst Index:%zd\n", id_src, srcIdx, id_dst, dstIdx);
        return NAME_NOT_FOUND;
    }

    ssize_t srcEdgeIdx = mUserVector.valueAt(srcIdx).mOut.indexOf(id_dst);
    ssize_t dstEdgeIdx = mUserVector.valueAt(dstIdx).mIn.indexOf(id_src);
    if (0 > srcEdgeIdx || 0 > dstEdgeIdx)
    {
        //MY_LOGE("Edge does not exist\nSrc ID:%" PRIxPTR " Src Index:%zd Dst ID:%" PRIxPTR " Dst Index:%zd", id_src, srcEdgeIdx, id_dst, dstEdgeIdx);
        return NAME_NOT_FOUND;
    }

    mUserVector.editValueAt(srcIdx).mOut.removeAt(srcEdgeIdx);
    mUserVector.editValueAt(dstIdx).mIn.removeAt(dstEdgeIdx);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
MyUserGraph::
setCapacity(size_t size)
{
    RWLock::AutoWLock _l(mRWLock);
    mUserVector.setCapacity(size);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
UsersManager::
MyUserGraph::
getGroupIndex() const
{
    RWLock::AutoRLock _l(mRWLock);
    return mGroupIndex;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
UsersManager::
MyUserGraph::
size() const
{
    RWLock::AutoRLock _l(mRWLock);
    return mUserVector.size();
}


/******************************************************************************
 *
 ******************************************************************************/
android::
sp<UsersManager::MyUser>
UsersManager::
MyUserGraph::
userAt(size_t index) const
{
    RWLock::AutoRLock _l(mRWLock);
    return mUserVector.valueAt(index).mMyUser;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
UsersManager::
MyUserGraph::
indegree(size_t index) const
{
    RWLock::AutoRLock _l(mRWLock);
    return mUserVector.valueAt(index).mIn.size();
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
UsersManager::
MyUserGraph::
outdegree(size_t index) const
{
    RWLock::AutoRLock _l(mRWLock);
    return mUserVector.valueAt(index).mOut.size();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
MyUserGraph::
setAcquireFence(
    size_t index,
    MINT fence
)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    sp<MyUser> const& pUser = mUserVector.valueAt(index).mMyUser;
    if  ( 0 <= pUser->mAcquireFence->getFd() ) {
        MY_LOGE("%zu: fail to set fence:%d since acquire fence:%d already exists", index, fence, pUser->mAcquireFence->getFd());
        return ALREADY_EXISTS;
    }
    //
    pUser->mAcquireFence = IFence::create(fence);
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MUINT
UsersManager::
MyUserGraph::
getCategory(
    size_t index
)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    sp<MyUser> const& pUser = mUserVector.valueAt(index).mMyUser;
    return pUser->mCategory;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
MyUserGraph::
getInUsers(size_t userId, android::Vector<android::sp<MyUser> >& result) const
{
    result.clear();
    UserNode const& usr = mUserVector.valueFor(userId);
    for (size_t i = 0; i < usr.mIn.size(); i++)
    {
        result.add(mUserVector.valueFor(usr.mIn.itemAt(i)).mMyUser);
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
MyUserGraph::
DFS(size_t userId, android::Vector<android::sp<MyUser> >& result) const
{
    ssize_t const idx = mUserVector.indexOfKey(userId);
    for (size_t i = 0; i < mUserVector.valueAt(idx).mIn.size(); i++)
    {
        if  ( 0 > DFS(mUserVector.valueAt(idx).mIn.itemAt(i), result) )
        {
            return UNKNOWN_ERROR;
        }
    }
    result.add(mUserVector.valueAt(idx).mMyUser);
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
MyUserGraph::
getPriorUsers(size_t userId, android::Vector<android::sp<MyUser> >& result) const
{
    result.clear();
    //
    //It also contains the current user.
    if(0 > DFS(userId, result))
    {
        return UNKNOWN_ERROR;
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
UsersManager::
UsersManager(Subject_T subject, char const* name)
    : mSubject(subject)
    , mSubjectName(name)
    , mRWLock()
    , mUserInit(MFALSE)
    , mUserGroupGraph()
    , mProducerMap()
    , mConsumerMap()
{
}


/******************************************************************************
 *
 ******************************************************************************/
IUsersManager::Subject_T
UsersManager::
getSubject() const
{
    return mSubject;
}


/******************************************************************************
 *
 ******************************************************************************/
char const*
UsersManager::
getSubjectName() const
{
    return mSubjectName.string();
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
UsersManager::
reset()
{
    RWLock::AutoWLock _l(mRWLock);
    //
    mUserInit = MFALSE;
    mUserGroupGraph.clear();
    mProducerMap.clear();
    mConsumerMap.clear();
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<UsersManager::MyUser>
UsersManager::
queryUser_(UserId_T const userId) const
{
    sp<MyUser> pUser;
    //
    if  (
            ( pUser = mProducerMap.valueFor(userId) ) == 0
        &&  ( pUser = mConsumerMap.valueFor(userId) ) == 0
        )
    {
        MY_LOGE(
            "Subject:%s cannot find userId:%#" PRIxPTR " #Producers:%zu #Consumers:%zu",
            getSubjectName(), userId, mProducerMap.size(), mConsumerMap.size()
        );
        NSCam::Utils::dumpCallStack(LOG_TAG);
        return NULL;
    }
    //
    if  ( (ssize_t)mUserGroupGraph.size() <= pUser->mGroupIndex ) {
        MY_LOGE(
            "userId:%p has groupIndex:%zu > mUserGroupGraph.size:%zu",
            userId, pUser->mGroupIndex, mUserGroupGraph.size()
        );
        return NULL;
    }
    return pUser;
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
UsersManager::
enqueUserGraph_(
    size_t groupIndex
)
{
    android::sp<MyUserGraph> myUserGraph = mUserGroupGraph.itemAt(groupIndex);
    //
    for (size_t i = 0; i < myUserGraph->size(); i++)
    {
        //
        android::sp<MyUser> pMyUser = myUserGraph->userAt(i);
        switch  (pMyUser->mCategory)
        {
        case Category::PRODUCER:{
            mProducerMap.add(pMyUser->mUserId, pMyUser);
            }break;
            //
        case Category::CONSUMER:{
            mConsumerMap.add(pMyUser->mUserId, pMyUser);
            myUserGraph->mConsumerUsage |= pMyUser->mUsage; //update usage of consumer group.
            }break;
            //
        default:
            break;
        }
        //
        MY_LOGD_IF(0, "Subject:%s userId:%#" PRIxPTR " Category:%d", getSubjectName(), pMyUser->mUserId, pMyUser->mCategory);
    }
    //
    return groupIndex;
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<UsersManager::MyUserGraph>
UsersManager::
queryUserGraph(
    IUserGraph*const pUserGraph
)   const
{
    if  ( ! pUserGraph ) {
        return NULL;
    }

    size_t const groupIndex = pUserGraph->getGroupIndex();

    RWLock::AutoRLock _l(mRWLock);
    if  ( mUserGroupGraph.size() <= groupIndex ) {
        MY_LOGW("groupIndex:%zu > %zu", groupIndex, mUserGroupGraph.size());
        return NULL;
    }
    return mUserGroupGraph[groupIndex];
}


/******************************************************************************
 *
 ******************************************************************************/
IUsersManager::
IUserGraph*
UsersManager::
createGraph()
{
    RWLock::AutoWLock _l(mRWLock);
    //
    if  ( mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] ALREADY_EXISTS", mSubject, getSubjectName());
        return NULL;
    }
    //
    size_t groupIndex = mUserGroupGraph.add();
    MyUserGraph *myUserGraph = new MyUserGraph(groupIndex);
    mUserGroupGraph.editItemAt(groupIndex) = myUserGraph;
    return myUserGraph;
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
UsersManager::
enqueUserGraph(
    android::sp<IUserGraph> pUserGraph
)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    if  ( mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] ALREADY_EXISTS", mSubject, getSubjectName());
        return ALREADY_EXISTS;
    }
    //
    return enqueUserGraph_(pUserGraph->getGroupIndex());
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
finishUserSetup()
{
    RWLock::AutoWLock _l(mRWLock);
    mUserInit = MTRUE;
    MY_LOGD_IF(0, "[subject:%#" PRIxPTR " %s]", mSubject, getSubjectName());
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
UsersManager::
markUserStatus(
    UserId_T userId,
    MUINT32 const statusMask
)
{
    BitSet32 bitset(statusMask);
    MUINT32 mask = statusMask;
    //
    RWLock::AutoWLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return 0;
    }
    //
    android::sp<MyUser> pMyUser = queryUser_(userId);
    if  ( pMyUser == 0 ) {
        return 0;
    }
    //
    for (MUINT32 bitmask = 1; 0 != mask; bitmask <<= 1)
    {
        //skip if not intending to be marked.
        if  ( bitmask != (bitmask & mask) ) {
            continue;
        }

        //clear this bit.
        mask &= ~ bitmask;

        //skip if marked before.
        if  ( bitmask == (bitmask & pMyUser->mUserStatus) ) {
            MY_LOGW_IF(
                0,
                "[subject:%#" PRIxPTR " userId:%#" PRIxPTR " userStatus:%#x] bit %#x/%#x has been marked",
                mSubject, userId, pMyUser->mUserStatus, bitmask, statusMask
            );
            continue;
        }

        //mark this bit.
        pMyUser->mUserStatus |= bitmask;

        //
        MyUserMap* pUserMap;
        if ( !pMyUser->mCategory ) {
            pUserMap = &mProducerMap;
        }
        else if ( 1 == pMyUser->mCategory ) {
            pUserMap = &mConsumerMap;
        }
        else
        {
            MY_LOGE("mCategory (%d)", pMyUser->mCategory);
            return 0;
        }

        switch  (bitmask)
        {
        case UserStatus::USED:
            pUserMap->mBitSetUsed.markBit(pUserMap->indexOfKey(userId));
            break;
            //
        case UserStatus::RELEASE:
            pUserMap->mBitSetReleased.markBit(pUserMap->indexOfKey(userId));
            break;
            //
        case UserStatus::PRE_RELEASE:
            pUserMap->mBitSetPreReleased.markBit(pUserMap->indexOfKey(userId));
            break;
            //
        default:
            break;
        }
    }
    //
    return pMyUser->mUserStatus;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
UsersManager::
getUserStatus(
    UserId_T userId
)   const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return 0;
    }
    //
    android::sp<MyUser> pMyUser = queryUser_(userId);
    if  ( pMyUser == 0 ) {
        return 0;
    }
    //
    return pMyUser->mUserStatus;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
UsersManager::
getUserCategory(
    UserId_T userId
)   const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return Category::NONE;
    }
    //
    android::sp<MyUser> pMyUser = queryUser_(userId);
    if  ( pMyUser == 0 ) {
        return Category::NONE;
    }
    //
    return pMyUser->mCategory;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
setUserReleaseFence(
    UserId_T userId,
    MINT releaseFence
)
{
    RWLock::AutoWLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return NO_INIT;
    }
    //
    android::sp<MyUser> pMyUser = queryUser_(userId);
    if  ( pMyUser == 0 ) {
        return NAME_NOT_FOUND;
    }
    //
    if  (
            pMyUser->mReleaseFence != 0
        &&  pMyUser->mReleaseFence->isValid()
        &&  pMyUser->mReleaseFence->getFd() == releaseFence
        )
    {
        MY_LOGW(
            "the same release fence:%s(%d) is set to userId:%p",
            pMyUser->mReleaseFence->name(),
            pMyUser->mReleaseFence->getFd(),
            userId
        );
        return ALREADY_EXISTS;
    }
    //
    pMyUser->mReleaseFence = IFence::create(releaseFence);
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
UsersManager::
queryGroupUsage(
    UserId_T userId
)   const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return 0;
    }
    //
    android::sp<MyUser> pMyUser = queryUser_(userId);
    if  ( pMyUser == 0 ) {
        return 0;
    }
    //
    MUINT groupUsage = 0;
    switch  (pMyUser->mCategory)
    {
    case Category::PRODUCER:
        groupUsage = pMyUser->mUsage;
        break;
        //
    case Category::CONSUMER:{
        size_t const groupIndex = pMyUser->mGroupIndex;
        if ( mUserGroupGraph.size() <= groupIndex ) {
            MY_LOGE(
                "[userId:%p] groupIndex:%zu > mUserGroupGraph.size:%zu",
                userId, groupIndex, mUserGroupGraph.size()
            );
        }
        groupUsage = mUserGroupGraph[groupIndex]->mConsumerUsage;
        }break;
        //
    default:
        break;
    }

    return groupUsage;
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
UsersManager::
getNumberOfProducers() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return 0;
    }
    //
    return mProducerMap.size();
}


/******************************************************************************
 *
 ******************************************************************************/
size_t
UsersManager::
getNumberOfConsumers() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return 0;
    }
    //
    return mConsumerMap.size();
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
UsersManager::
createAcquireFence(
    UserId_T userId
)   const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return -1;
    }
    //
    android::sp<MyUser> pMyUser = queryUser_(userId);
    if  ( pMyUser == 0 ) {
        return -1;
    }
    size_t groupIndex = pMyUser->mGroupIndex;
    //
    IFencePtr_T fence;
    size_t g = 0;
    if  ( groupIndex == g) {
        android::sp<MyUserGraph> myUserGraph = mUserGroupGraph[g];
        android::Vector<android::sp<MyUser> > result;
        if (0 > myUserGraph->getPriorUsers(userId, result))
        {
            return UNKNOWN_ERROR;
        }
        for (size_t j = 0; j < result.size(); j++) {
            android::sp<MyUser> myUser = result.itemAt(j);
            if  ( userId == myUser->mUserId ) {
                if  ( 0 == g ) {  //1st users @ 1st group
                    IFencePtr_T AF = myUser->mAcquireFence;
                    if  ( AF != 0 ) {
                        if ( fence==0 ) {
                        return AF->dup();
                        } else {
                            android::String8 name = android::String8::format("%s-%s", fence->name(), AF->name());
                            IFencePtr_T AF_priorRFs = IFence::merge(name.string(), fence, AF);
                            return AF_priorRFs->dup();
                        }
                    }
                    else {
                        if ( fence!=0 ) {
                            return fence->dup();
                        }
                        return -1;
                    }
                }
                else {
                    return fence->dup();
                }
            }
            //
            IFencePtr_T RF = myUser->mReleaseFence;
            if  ( RF == 0 ) {
                continue;
            }
            //
            if  ( fence == 0 ) {
                fence = IFence::create(RF->dup());
            }
            else {
                android::String8 name = android::String8::format("%s-%s", fence->name(), RF->name());
                fence = IFence::merge(name.string(), fence, RF);
            }
        }
    }
    //
    MY_LOGE("Never here...something wrong!!!");
    return -1;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT
UsersManager::
createReleaseFence(
    UserId_T userId
)   const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return -1;
    }
    //
    android::sp<MyUser> pMyUser = queryUser_(userId);
    if  ( pMyUser == 0 ) {
        return -1;
    }
    //
    IFencePtr_T RF = pMyUser->mReleaseFence;
    if  ( RF == 0 ) {
        MY_LOGW("subject:%#" PRIxPTR " userId:%#" PRIxPTR " has no release fence", mSubject, userId);
        return -1;
    }
    //
    return RF->dup();
}

/******************************************************************************
 *
 ******************************************************************************/
MINT
UsersManager::
createAcquireFence() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return -1;
    }
    //
    IFencePtr_T fence;
    for (size_t g = 0; g < mUserGroupGraph.size(); g++) {
        android::sp<MyUserGraph> myUserGraph = mUserGroupGraph[g];
        if ( ! myUserGraph.get() )
            continue;
        for (size_t i = 0; i < myUserGraph->size(); i++) {
            android::sp<MyUser> myUser = myUserGraph->userAt(i);
            //
            IFencePtr_T AF = myUser->mAcquireFence;
            if  ( AF == 0 || AF->getFd() == -1 ) {
                continue;
            } else {
                android::String8 name = android::String8::format("%s", AF->name());
                return AF->dup();
            }
        }
    }
    //
    return -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
UsersManager::
createReleaseFence() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return -1;
    }
    //
    IFencePtr_T fence;
    for (size_t g = 0; g < mUserGroupGraph.size(); g++) {
        android::sp<MyUserGraph> myUserGraph = mUserGroupGraph[g];
        if ( ! myUserGraph.get() )
            continue;
        for (size_t i = 0; i < myUserGraph->size(); i++) {
            android::sp<MyUser> myUser = myUserGraph->userAt(i);
            //
            IFencePtr_T RF = myUser->mReleaseFence;
            if  ( RF == 0 ) {
                continue;
            }
            //
            if  ( fence == 0 ) {
                fence = IFence::create(RF->dup());
            }
            else {
                android::String8 name = android::String8::format("%s-%s", fence->name(), RF->name());
                fence = IFence::merge(name.string(), fence, RF);
            }
        }
    }
    //
    return (fence != 0) ? fence->dup() : -1;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
haveAllUsersReleasedOrPreReleased(
    UserId_T userId
)   const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return NO_INIT;
    }
    //
    android::sp<MyUser> pMyUser = queryUser_(userId);
    if  ( pMyUser == 0 ) {
        return NAME_NOT_FOUND;
    }
    size_t const groupIndex = pMyUser->mGroupIndex;
    //
    size_t g = 0;
    if  ( groupIndex == g) {
        android::sp<MyUserGraph> myUserGraph = mUserGroupGraph[g];
        android::Vector<android::sp<MyUser> > result;
        MERROR err = myUserGraph->getPriorUsers(userId, result);
        if  ( OK != err ) {
            MY_LOGE("[Subject:%#" PRIxPTR "] userId:%#" PRIxPTR " getPriorUsers return %d", mSubject, userId, err);
            return err;
        }
        for (size_t j = 0; j < result.size(); j++) {
            android::sp<MyUser>const& pThisUser = result[j];
            //
            if  ( userId == pThisUser->mUserId ) {
                continue;
            }
            //
            if  ( Category::NONE == pThisUser->mCategory ) {
                continue;
            }
            //
            bool const isPreReleased= pThisUser->mUserStatus & (UserStatus::PRE_RELEASE);
            bool const isReleased   = pThisUser->mUserStatus & (UserStatus::RELEASE);
            if  ( ! isReleased && ! isPreReleased ) {
                MY_LOGW_IF(
                    1,
                    "[Subject:%#" PRIxPTR "] UserId:%#" PRIxPTR " ahead of UserId:%#" PRIxPTR " has not released:%d or pre-released:%d",
                    mSubject, pThisUser->mUserId, userId, isReleased, isPreReleased
                );
                return NO_INIT;
            }
        }
        return OK;
    }
    //
    MY_LOGE("[Subject:%#" PRIxPTR "] UserId:%#" PRIxPTR " groupIndex=%zu", mSubject, userId, groupIndex);
    return UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
haveAllUsersReleased() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    return haveAllUsersReleasedLocked();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
haveAllUsersReleasedLocked() const
{
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return NO_INIT;
    }
    //
    if  (
            mConsumerMap.size() == mConsumerMap.mBitSetReleased.count()
        &&  mProducerMap.size() == mProducerMap.mBitSetReleased.count()
        )
    {
        MY_LOGD_IF(
            0,
            "[subject:%#" PRIxPTR "] "
            "[Consumer] size:%zu #released:%#x "
            "[Producer] size:%zu #released:%#x ",
            mSubject,
            mConsumerMap.size(), mConsumerMap.mBitSetReleased.value,
            mProducerMap.size(), mProducerMap.mBitSetReleased.value
        );
        return OK;
    }
    //
    return UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
haveAllUsersReleasedOrPreReleasedLocked() const
{
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return NO_INIT;
    }
    //
    BitSet32 const consumer = mConsumerMap.mBitSetReleased | mConsumerMap.mBitSetPreReleased;
    if  ( consumer.count() != mConsumerMap.size() ) {
        return UNKNOWN_ERROR;
    }
    //
    BitSet32 const producer = mProducerMap.mBitSetReleased | mProducerMap.mBitSetPreReleased;
    if  ( producer.count() != mProducerMap.size() ) {
        return UNKNOWN_ERROR;
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT32
UsersManager::
getAllUsersStatus() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if ( OK == haveAllUsersReleasedLocked() ) {
        return UserStatus::RELEASE;
    }
    else if( OK == haveAllUsersReleasedOrPreReleasedLocked() ) {
        return UserStatus::PRE_RELEASE;
    }
    //
    return 0;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
haveAllProducerUsersReleased() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return NO_INIT;
    }
    //
    return  mProducerMap.size() == mProducerMap.mBitSetReleased.count()
        ?   OK
        :   UNKNOWN_ERROR
            ;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
UsersManager::
haveAllProducerUsersUsed() const
{
    RWLock::AutoRLock _l(mRWLock);
    //
    if  ( ! mUserInit ) {
        MY_LOGW("[subject:%#" PRIxPTR " %s] NO_INIT", mSubject, getSubjectName());
        return NO_INIT;
    }
    //
    return  mProducerMap.size() == mProducerMap.mBitSetUsed.count()
        ?   OK
        :   UNKNOWN_ERROR
            ;
}

