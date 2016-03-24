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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_STREAM_IUSERSMANAGER_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_STREAM_IUSERSMANAGER_H_
//
#include <utils/RefBase.h>
#include <mtkcam/common.h>


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


/**
 * An interface of subject users manager.
 * It is designed for managing a specific subject and its users.
 */
class IUsersManager
    : public virtual android::RefBase
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Definitions.

    /**
     * The type of subject to use.
     */
    typedef MINTPTR                     Subject_T;

    /**
     * The type of user Id.
     */
    typedef MINTPTR                     UserId_T;

    /**
     * User Category.
     */
    struct Category
    {
        enum
        {
            PRODUCER                    = 0,    //this user is a producer
            CONSUMER                    = 1,    //this user is a consumer
            NONE                        = 2,    //this is not a valid user
        };
    };

    /**
     * User.
     */
    struct  User
    {
        /**
         * A unique user Id.
         */
        UserId_T                        mUserId;

        /**
         * Acquire fence.
         * The callee takes the ownership of the fence file descriptor and is
         * charge of closing it.
         * This user must wait on this fence before using its associated subject.
         */
        MINT                            mAcquireFence;

        /**
         * Release fence.
         * The callee takes the ownership of the fence file descriptor and is
         * charge of closing it.
         * This fence will be signaled after its associated subject is used done
         * by this user.
         */
        MINT                            mReleaseFence;

        /**
         * Usage.
         */
        MUINT                           mUsage;

        /**
         * User category
         * Category::PRODUCER
         * Category::CONSUMER
         * Category::NONE
         */
        MUINT                           mCategory;

        /**
         * Constructor.
         */
                                        User()
                                            : mUserId(-1)
                                            , mAcquireFence(-1)
                                            , mReleaseFence(-1)
                                            , mUsage(0)
                                            , mCategory(0)
                                        {}
    };


    class IUserGraph
        : public virtual android::RefBase
    {
    public:

        /**
         * Add a user to the graph.
         *
         * @param[in] usr: a user.
         *
         * @return
         *      0 indicates success; otherwise failure.
         */
        virtual MERROR                  addUser(
                                            User const& usr
                                        )                                   = 0;

        /**
         * Remove a node and its associated edge.
         *
         * @param[in] id: node id
         *
         * @return
         *      0 indicates success; otherwise failure.
         */
        virtual MERROR                  removeUser(
                                            UserId_T id
                                        )                                   = 0;

        /**
         * Add a directed edge to the graph, where a directed edge is from a
         * given source node to a given destination node.
         *
         * @param[in] id_src: the node id of source.
         *
         * @param[in] id_dst: the node id of destination.
         *
         * @return
         *      0 indicates success; otherwise failure.
         */
        virtual MERROR                  addEdge(
                                            UserId_T id_src,
                                            UserId_T id_dst
                                        )                                   = 0;

        /**
         * Remove a directed edge from the graph, where a directed edge is from
         * a given source node to a given destination node.
         *
         * @param[in] id_src: the node id of source.
         *
         * @param[in] id_dst: the node id of destination.
         *
         * @return
         *      0 indicates success; otherwise failure.
         */
        virtual MERROR                  removeEdge(
                                            UserId_T id_src,
                                            UserId_T id_dst
                                        )                                   = 0;

        /**
         * Set capacity of vector inside
         */
        virtual MERROR                  setCapacity(
                                            size_t size
                                        )                                   = 0;

        /**
         * Get the number of users.
         */
        virtual size_t                  size() const                        = 0;

        /**
         * Get the in-coming number of users.
         */
        virtual size_t                  indegree(size_t index) const        = 0;

        /**
         * Get the out-going number of users.
         */
        virtual size_t                  outdegree(size_t index) const       = 0;

        /**
         * Set a specified user's acquire fence.
         */
        virtual MERROR                  setAcquireFence(
                                            size_t index,
                                            MINT fence
                                        )                                   = 0;

        /**
         * Get a specified user's category.
         */
        virtual MUINT                   getCategory(
                                            size_t index
                                        )                                   = 0;

        /**
         * Get group index of graph
         */
        virtual size_t                  getGroupIndex() const               = 0;
    };

    /**
     *  ACQUIRE     : This user has waited on the subject's acquire fence.
     *  PRE_RELEASE : This user is ready to pre-release the subject and will
     *                still use it after its pre-release until a release fence
     *                is signalled.
     *  USED        : This user has used the subject.
     *  RELEASE     : This user is ready to release the subject and will not use
     *                it after its release.
     */
    struct UserStatus
    {
        enum
        {
            ACQUIRE                     = (0x00000001 << 0),
            PRE_RELEASE                 = (0x00000001 << 1),
            USED                        = (0x00000001 << 2),
            RELEASE                     = (0x00000001 << 3),
        };
    };


public:     ////                        Operations.

    /**
     * Return the subject id.
     */
    virtual Subject_T                   getSubject() const                  = 0;

    /**
     * Return the subject name.
     */
    virtual char const*                 getSubjectName() const              = 0;

public:     ////                        Operations.

    /**
     * Create a user graph.
     */
    virtual IUserGraph*                 createGraph()                       = 0;

    /**
     * Enqueue a specified user graph to a specified group.
     *
     * A user group could contain several user graphs, and a user graph could
     * contain several users.
     *
     * @param[in] pUserGraph: A specified user graph.
     *
     * @return
     *      >=0: an index of user group where the user graph is added into.
     *      < 0: an error code.
     */
    virtual ssize_t                     enqueUserGraph(
                                            android::sp<IUserGraph> pUserGraph
                                        )                                   = 0;

    /**
     * Completes the user setup process.
     *
     * @return
     *      0 indicates success; otherwise failure.
     */
    virtual MERROR                      finishUserSetup()                   = 0;

    /**
     * Reset.
     */
    virtual MVOID                       reset()                             = 0;

public:     ////                        Operations.

    /**
     * This call marks a specified user's status.
     *
     * @param[in] userId: A specified unique user Id.
     *
     * @param[in] statusMask: User status.
     *      It's available to mark two status. (ex. 0x0001, 0x1100, ...)
     *
     * @return
     *      the current status mask.
     */
    virtual MUINT32                     markUserStatus(
                                            UserId_T userId,
                                            MUINT32 const statusMask
                                        )                                   = 0;

    /**
     * Retrieve a specified user's status mask.
     *
     * @param[in] userId: A specified unique user Id.
     *
     * @return
     *      the current status mask.
     */
    virtual MUINT32                     getUserStatus(
                                            UserId_T userId
                                        )   const                           = 0;

    /**
     * Retrieve a specific user's category.
     *
     * @param[in] userId: A specified unique user Id.
     *
     * @return
     *      the user's category (Category::xxx).
     */
    virtual MUINT                       getUserCategory(
                                            UserId_T userId
                                        )   const                           = 0;

    /**
     * Set a specified user's release fence.
     * The specified user must be enqueued before this call.
     *
     * @param[in] userId: A specified unique user Id.
     *
     * @param[in] releaseFence: A release fence to register.
     *      The callee takes the ownership of the fence file descriptor and is
     *      charge of closing it.
     *      If a release fence associated with this user is specified during
     *      enqueUserGraph(), the old release fence will be replaced with the
     *      specified release fence after this call.
     *
     * @return
     *      0 indicates success; otherwise failure.
     *      NAME_NOT_FOUND indicates a bad user Id.
     */
    virtual MERROR                      setUserReleaseFence(
                                            UserId_T userId,
                                            MINT releaseFence
                                        )                                   = 0;

    /**
     * Query a specific user's group usage.
     *
     * @param[in] userId: A specified unique user Id.
     *
     * @return
     *      A group usage associated with this user.
     */
    virtual MUINT                       queryGroupUsage(
                                            UserId_T userId
                                        )   const                           = 0;

    /**
     * Retrieve the number of producer users.
     */
    virtual size_t                      getNumberOfProducers() const        = 0;

    /**
     * Retrieve the number of consumer users.
     */
    virtual size_t                      getNumberOfConsumers() const        = 0;

public:     ////                        Operations.

    /**
     * Create a subject's acquire fence associated with a user.
     * This user must wait on this fence before attempting to use the subject.
     *
     * @param[in] userId: A specified user Id.
     *
     * @return
     *      A bad fence indicates this subject has not been initialized or need
     *      not to wait before using it.
     */
    virtual MINT                        createAcquireFence(
                                            UserId_T userId
                                        )   const                           = 0;

    /**
     * Create a subject's release fence associated with a user.
     * The signaling of this fence indicates that this subject is accessed done
     * and released by this user
     *
     * @param[in] userId: A specified user Id.
     *
     * @return
     *      A bad fence indicates this subject has not been initialized or need
     *      not to wait before using it.
     */
    virtual MINT                        createReleaseFence(
                                            UserId_T userId
                                        )   const                           = 0;

    /**
     * get a subject's acquire fence from framework if it exists.
     *
     * @return
     *      A bad fence indicates this subject has not been initialized or need
     *      not to wait before accessing it.
     */
    virtual MINT                        createAcquireFence(
                                        )   const                           = 0;

    /**
     * Create a subject's final release fence.
     * The signaling of this fence indicates that no more users attempt to
     * use this given subject.
     *
     * @return
     *      A bad fence indicates this subject has not been initialized or need
     *      not to wait before accessing it.
     */
    virtual MINT                        createReleaseFence(
                                        )   const                           = 0;

public:     ////                        Operations.

    /**
     * return OK if all users ahead of this user are released or pre-released.
     */
    virtual MERROR                      haveAllUsersReleasedOrPreReleased(
                                            UserId_T userId
                                        )   const                           = 0;

    /**
     *
     */
    virtual MERROR                      haveAllUsersReleased(
                                        )   const                           = 0;

    /**
     * return UserStatus::RELEASE
             if all users are released

       return UserStatus::PRE_RELEASE
             if all users are released or pre-released.

       return 0
             if NOT all users are released or pre-released.
     */
    virtual MUINT32                     getAllUsersStatus() const           = 0;


    /**
     *
     */
    virtual MERROR                      haveAllProducerUsersReleased(
                                        )   const                           = 0;

    /**
     *
     */
    virtual MERROR                      haveAllProducerUsersUsed() const    = 0;
};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace v3
};  //namespace NSCam
#endif  //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_STREAM_IUSERSMANAGER_H_

