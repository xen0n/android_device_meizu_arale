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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_USERSMANAGER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_USERSMANAGER_H_
//
#include <utils/BitSet.h>
#include <utils/RWLock.h>
#include <utils/KeyedVector.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/Sync.h>
#include <mtkcam/v3/stream/IUsersManager.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {
namespace Utils {


/**
 * An implementation of subject users manager.
 */
class UsersManager
    : public virtual IUsersManager
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Definitions.

    /**
     * The type of a fence in order to sync. accessing a given subject.
     */
    typedef android::sp<
        NSCam::Utils::Sync::IFence
    >                                   IFencePtr_T;

    struct MyUser
        : public android::LightRefBase<MyUser>
    {
        UserId_T                        mUserId;
        IFencePtr_T                     mAcquireFence;
        IFencePtr_T                     mReleaseFence;
        MUINT                           mUsage;
        MUINT                           mCategory;
        ssize_t                         mGroupIndex;
        MUINT32                         mUserStatus;
                                        //
                                        MyUser(
                                            User const& user,
                                            ssize_t groupIndex
                                        );
    };

    class MyUserGraph : public IUsersManager::IUserGraph
    {
        friend  class UsersManager;
    public:     ////                    Definitions.

        struct UserNode
        {
            android::sp<MyUser>         mMyUser;

            /**
            * In-coming edges of this node
            */
            android::SortedVector<UserId_T>
                                        mIn;

            /**
            * Out-going edges of this node
            */
            android::SortedVector<UserId_T>
                                        mOut;

                                        UserNode(android::sp<MyUser> pUser = 0)
                                            : mMyUser(pUser)
                                        {}
        };

    protected:  ////                    Data Members.
        mutable android::RWLock         mRWLock;
        MUINT                           mConsumerUsage;
        size_t                          mGroupIndex;
        android::KeyedVector<
            UserId_T, UserNode
        >                               mUserVector;

    public:     ////                    Operations.
                                        MyUserGraph(
                                            size_t groupIndex
                                        );

        virtual MERROR                  addUser(
                                            User const& usr
                                        );

        virtual MERROR                  removeUser(
                                            UserId_T id
                                        );

        virtual MERROR                  addEdge(
                                            UserId_T id_src,
                                            UserId_T id_dst
                                        );

        virtual MERROR                  removeEdge(
                                            UserId_T id_src,
                                            UserId_T id_dst
                                        );

        virtual MERROR                  setCapacity(
                                            size_t size
                                        );

        virtual size_t                  getGroupIndex() const;

        virtual size_t                  size() const;

        virtual size_t                  indegree(size_t index) const;

        virtual size_t                  outdegree(size_t index) const;

        android::sp<MyUser>             userAt(size_t index) const;

        MERROR                          setAcquireFence(
                                            size_t index,
                                            MINT fence
                                        );

        MUINT                           getCategory(
                                            size_t index
                                        );

        MERROR                          getInUsers(
                                            size_t userId,
                                            android::Vector<android::sp<MyUser> >& result
                                        )   const;

        MERROR                          DFS(
                                            size_t userId,
                                            android::Vector<android::sp<MyUser> >& result
                                        )   const;

        MERROR                          getPriorUsers(
                                            size_t userId,
                                            android::Vector<android::sp<MyUser> >& result
                                        )   const;
    };

    struct MyUserGroupGraph
        : public android::Vector< android::sp<MyUserGraph> >
    {
        //Override clear() if additional members are added.
    };

    struct MyUserMap
        : public android::DefaultKeyedVector<UserId_T, android::sp<MyUser> >
    {
        typedef android::DefaultKeyedVector<UserId_T, android::sp<MyUser> >
                                        ParentT;
        android::BitSet32               mBitSetUsed;
        android::BitSet32               mBitSetReleased;
        android::BitSet32               mBitSetPreReleased;

        MVOID                           clear()
                                        {
                                            ParentT::clear();
                                            mBitSetUsed.clear();
                                            mBitSetReleased.clear();
                                            mBitSetPreReleased.clear();
                                        }
    };

private:    ////                        Data Members.
    Subject_T const                     mSubject;
    android::String8                    mSubjectName;
    mutable android::RWLock             mRWLock;
    MBOOL                               mUserInit;
    MyUserGroupGraph                    mUserGroupGraph;
    MyUserMap                           mProducerMap;
    MyUserMap                           mConsumerMap;

public:     ////                        Operations.
                                        UsersManager(Subject_T subject, char const* name = "");

protected:  ////                        Operations.
    android::sp<MyUser>                 queryUser_(UserId_T const userId) const;

    ssize_t                             enqueUserGraph_(
                                            size_t groupIndex
                                        );

    android::sp<MyUserGraph>            queryUserGraph(
                                            IUserGraph*const pUserGraph
                                        )   const;

    MERROR                              haveAllUsersReleasedLocked() const;
    MERROR                              haveAllUsersReleasedOrPreReleasedLocked() const;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IUsersManager Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Operations.
    virtual Subject_T                   getSubject() const;
    virtual char const*                 getSubjectName() const;

public:     ////                        Operations.

    virtual IUsersManager::IUserGraph*  createGraph();

    virtual ssize_t                     enqueUserGraph(
                                            android::sp<IUserGraph> pUserGraph
                                        );

    virtual MERROR                      finishUserSetup();

    virtual MVOID                       reset();

public:     ////                        Operations.

    virtual MUINT32                     markUserStatus(
                                            UserId_T userId,
                                            MUINT32 const statusMask
                                        );

    virtual MUINT32                     getUserStatus(
                                            UserId_T userId
                                        )   const;

    virtual MUINT                       getUserCategory(
                                            UserId_T userId
                                        )   const;

    virtual MERROR                      setUserReleaseFence(
                                            UserId_T userId,
                                            MINT releaseFence
                                        );

    virtual MUINT                       queryGroupUsage(
                                            UserId_T userId
                                        )   const;

    virtual size_t                      getNumberOfProducers() const;
    virtual size_t                      getNumberOfConsumers() const;

public:     ////                        Operations.

    virtual MINT                        createAcquireFence(
                                            UserId_T userId
                                        )   const;

    virtual MINT                        createReleaseFence(
                                            UserId_T userId
                                        )   const;

    virtual MINT                        createAcquireFence() const;

    virtual MINT                        createReleaseFence() const;

public:     ////                        Operations.

    /**
     * return OK if all users ahead of this user are released or pre-released.
     */
    virtual MERROR                      haveAllUsersReleasedOrPreReleased(
                                            UserId_T userId
                                        )   const;
    // All User
    virtual MERROR                      haveAllUsersReleased() const;

    /**
     * return UserStatus::RELEASE
             if all users are released

       return UserStatus::PRE_RELEASE
             if all users are released or pre-released.

       return 0
             if NOT all users are released or pre-released.
     */
    virtual MUINT32                     getAllUsersStatus() const;

    // All Producer
    virtual MERROR                      haveAllProducerUsersReleased() const;

    virtual MERROR                      haveAllProducerUsersUsed() const;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace Utils
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_UTILS_STREAMBUF_USERSMANAGER_H_

