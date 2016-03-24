/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/CamNG"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/camnode/ICamGraphNode.h>
//
#include <utils/Mutex.h>
using namespace android;
#include <list>
#include <vector>
#include <queue>
using namespace std;
//
#include <sys/resource.h>
//
#include <semaphore.h>
#include <utils/Thread.h>
#include <cutils/properties.h>
using namespace android;

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)       CAM_LOGV("[%d:%s::%s] " fmt, getSensorIdx(), getName(),  __func__, ##arg)
#define MY_LOGD(fmt, arg...)       CAM_LOGD("[%d:%s::%s] " fmt, getSensorIdx(), getName(),  __func__, ##arg)
#define MY_LOGI(fmt, arg...)       CAM_LOGI("[%d:%s::%s] " fmt, getSensorIdx(), getName(),  __func__, ##arg)
#define MY_LOGW(fmt, arg...)       CAM_LOGW("[%d:%s::%s] " fmt, getSensorIdx(), getName(),  __func__, ##arg)
#define MY_LOGE(fmt, arg...)       CAM_LOGE("[%d:%s::%s] " fmt, getSensorIdx(), getName(),  __func__, ##arg)
#define MY_LOGA(fmt, arg...)       CAM_LOGA("[%d:%s::%s] " fmt, getSensorIdx(), getName(),  __func__, ##arg)
#define MY_LOGF(fmt, arg...)       CAM_LOGF("[%d:%s::%s] " fmt, getSensorIdx(), getName(),  __func__, ##arg)

#define MY_ASSERT( exp ) do{if(!(exp)) { MY_LOGE(#exp); return MFALSE; }}while(0)
#define MY_ASSERT_NODE_OP( ret, pNode, op )          \
    do{                                              \
        if(!( ret = (pNode)->op() ) )                \
        {                                            \
            MY_LOGE("%s %s failed",                  \
                    pNode->getName(), #op );         \
            goto lbExit;                             \
        }                                            \
    }while(0)
#define MY_ASSERT_EXIT( exp )                        \
    do{                                              \
        if(!( exp ))                                 \
        {                                            \
            MY_LOGE(#exp);                           \
            goto lbExit;                             \
        }                                            \
    }while(0)

#define MY_ASSERT_STATE( exp, state )                \
    do{                                              \
        if(!(exp))                                   \
        {                                            \
            MY_LOGE("%s: state(0x%x)", #exp, state); \
            return MFALSE;                           \
        }                                            \
    }while(0)

//
/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

#define GET_NOTIFYCATEGORY(msg)            ( (msg) & 0x0000000F )
#define HAVE_NOTIFY(a,b)                   ( (GET_NOTIFYCATEGORY(a) == GET_NOTIFYCATEGORY(b)) &&  \
                                           ( (a & b) == b) )

/*******************************************************************************
*
********************************************************************************/
typedef enum GraphNodeState{
    State_Idle      = 0x1,   //after create or after disconnect
    State_Connected = 0x2,   //after connect or after uninit
    State_Initiated = 0x4,   //after init or after stop
    State_Started   = 0x8,   //after start
    State_Paused    = 0x10,  //after pause //TODO
    State_Error     = 0x100, //error
} GraphNodeState;

/*******************************************************************************
*  |  -->  |    connect        init          Start        |
*  | State | Idle     connected     initiated     Started |
*  |  <--  |    disconnect     uninit        Stop         |
********************************************************************************/


typedef enum ThreadCmdType{
    TCmd_Sync     = 0x1,
    TCmd_Init     = 0x2,
    TCmd_Update   = 0x4,
    TCmd_Uninit   = 0x8,
    TCmd_Exit     = 0x10,
} ThreadCmdType;


enum{
    NODE_CONNECT_AS_SRC      = 0x1,
    NODE_CONNECT_AS_DST      = 0x2,
    NODE_CONNECT_AS_SINGLE   = 0x4,
};

/*******************************************************************************
*
********************************************************************************/
class CamBufMgr
{
    public:   ////    Constructor/Destructor.
        CamBufMgr(MINT32 const sensorIdx);
        virtual         ~CamBufMgr();

    public:     ////    Instantiation.


    public:     ////    Operations.

        MBOOL   reset();

        ICamBufHandler*   getBufferHandler( MUINT32 const data ) const;
        MBOOL             setBufferHandler( MUINT32 const data, ICamBufHandler * const pBufHdl);

        MBOOL             requestBuffer(
                                MUINT32 const data,
                                AllocInfo const & info
                                ) const;

        MBOOL             dequeBuffer(
                                MUINT32 const data,
                                ImgRequest * pImgReq
                                ) const;

        MBOOL             enqueBuffer(
                                MUINT32 const data,
                                IImageBuffer const * pImageBuffer
                                ) const;

        MINT32 getSensorIdx() const { return miSensorIdx; }

    protected:     ////    Info

        const char* getName() const { return msName; }

    private:

        const char* const                   msName;
        MINT32 const                        miSensorIdx;

        typedef struct {
            vector<MUINT32> mvData;
            ICamBufHandler* pHandle;
        } BufHdlMap;

        mutable Mutex                       mLock;
        vector< BufHdlMap >                 mvBufHdl;
};


/*******************************************************************************
*
********************************************************************************/
class ICamNodeImpl
{
    friend class ICamThreadImpl;
    public:

        ICamNodeImpl();
        ICamNodeImpl(
                const char* name,
                ICamNode* const pNode
                );
        virtual ~ICamNodeImpl();

        //src_dst: src(0), dst(1)
        MBOOL addDataSupport(MBOOL const src_dst, MUINT32 const data);
        MBOOL addNotifySupport(MUINT32 const msg);
        MBOOL isDataConnected(MUINT32 const data) const;
        vector<MUINT32> getDataConnected() const        {   return mvDataDst;   }
        MBOOL checkDataSupport(MUINT32 connect_type, MUINT32 data) const;
        MBOOL checkNotifySupport(MUINT32 data) const;

        MBOOL connectData(
                MUINT32 const connect_type,
                ICamGraphImpl* const graph_impl,
                MUINT32 const srcdata,
                MUINT32 const dstdata,
                ICamNodeImpl* const pNode_impl
                );
        MBOOL connectNotify(
                ICamGraphImpl* const graph_impl,
                MUINT32 const msg,
                ICamNodeImpl* const pNode_impl
                );
        MBOOL disconnect();
virtual MBOOL init();
virtual MBOOL uninit();
virtual MBOOL start();
virtual MBOOL stop();
        MBOOL stopPostBuffer();
        //MBOOL Pause();
        //MBOOL Resume();

        MUINT32 getState() const { Mutex::Autolock _l(mLock); return mState; }

        MBOOL Notify(  //FIXME: remove this
                MUINT32 const msg,
                MUINT32 const ext1,
                MUINT32 const ext2
                );

        MBOOL setCamBufMgr(CamBufMgr* bufmgr);
        ICamBufHandler* getBufferHandler(MUINT32 const data) const;

        MBOOL handlePostBuffer(
                MUINT32 const data,
                MUINTPTR const buf,
                MUINT32 const ext
                ) const;

        MBOOL handleReturnBuffer(
                MUINT32 const data,
                MUINTPTR const buf,
                MUINT32 const ext
                ) const;

        MBOOL handleNotify(
                MUINT32 const msg,
                MUINT32 const ext1 = 0,
                MUINT32 const ext2 = 0
                ) const;

        MBOOL handleBroadcast(
                MUINT32 const msg,
                MUINT32 const ext1 = 0,
                MUINT32 const ext2 = 0
                ) const;

        const char* getName() const { return msName; }

        MINT32     getSensorIdx() const { return miSensorIdx; }
        MVOID      setSensorIdx(MINT32 const sensorIdx) { if( miSensorIdx == -1 ) miSensorIdx = sensorIdx; }

    private:
        typedef struct {
            MUINT32 srcdata;
            MUINT32 dstdata;
            ICamNodeImpl* nodeImpl;
        } DataLink;

        typedef struct {
            ICamNodeImpl*       mpNodeImpl;
            list<MUINT32>       mlMsg;
        } NotifyLink;

        MBOOL getDataNode(
                MUINT32 const type,
                MUINT32 const data,
                ICamNodeImpl** ppNode,
                MUINT32* pData
                ) const;

        MVOID dumpDataLink() const;

        MBOOL checkDumpData(MUINT32 const data) const;

    protected:

        ICamNode* const                     mpSelf;
        mutable Mutex                       mLock;

    private:
        const char* const                   msName;
        MINT32                              miSensorIdx;
        ICamGraphImpl*                      mpGraphImpl;
        CamBufMgr*                          mpBufMgr;

        vector<MUINT32>                     mvFlagSrc;
        vector<MUINT32>                     mvFlagDst;
        vector<MUINT32>                     mvNotify;

        MUINT32                             mState;
        MBOOL                               mbStopPostBuffer;

        //data
        vector<MUINT32>                     mvDataDst;
        list<DataLink>                      mlDataLink;
        //notify
        list<NotifyLink>                    mlNotifyLink;


};


/*******************************************************************************
*
********************************************************************************/
class ICamGraphImpl
{
    public:

        ICamGraphImpl(
                MINT32 const sensorIdx,
                const char* name,
                ICamGraph* const pGraph
                );
        ~ICamGraphImpl();

        MBOOL connectData(
                MUINT32 const updata,
                MUINT32 const downdata,
                ICamNode* const pUpNode,
                ICamNode* const pDownNode
                );

        MBOOL connectNotify(
                MUINT32 const msg,
                ICamNode* const pUpNode,
                ICamNode* const pDownNode
                );

        MBOOL disconnect();
        MBOOL init();
        MBOOL uninit();
        MBOOL start();
        MBOOL stop();
        MBOOL broadcast(
                MUINT32 const msg,
                MUINT32 const ext1,
                MUINT32 const ext2
                );

        //MBOOL ReplaceNode(ICamNode* const pOld, ICamNode* const pNew);
        MBOOL setBufferHandler(MUINT32 const data, ICamBufHandler* bufHdl, MINT32 idx = -1);

        MBOOL addToNodeList(ICamNodeImpl* up, ICamNodeImpl* down);

        const char* getName() const { return msName; }

        MINT32      getSensorIdx() const { return miSensorIdx; }

        MBOOL       isDumpEnable() const { return mbDumpEnable; }

    protected:

        ICamGraph* const        mpSelf;

    private:
        mutable Mutex           mLock;
        const char* const       msName;
        MINT32 const            miSensorIdx;
        MUINT32                 mState;
        vector< CamBufMgr* >    mvpBufMgr;
        MBOOL                   mbDumpEnable;

        vector< ICamNodeImpl* > mvNodeImpls;
};


/*******************************************************************************
*
********************************************************************************/
class ICamThreadImpl : public ICamNodeImpl
{
    class ThreadHandle;

    public:

        ICamThreadImpl(
                const char* name,
                ICamThreadNode* const pNode,
                eThreadNodeType const type,
                MUINT32 policy,
                MUINT32 priority
                );
        virtual ~ICamThreadImpl();

    public:

        virtual MBOOL init();
        virtual MBOOL uninit();
        virtual MBOOL start();
        virtual MBOOL stop();

        MBOOL             handleThreadInit() const;
        MBOOL             handleThreadUninit() const;
        MBOOL             handleLoopUpdate() const;
        eThreadNodeType   getThreadNodeType() const;

        MBOOL             syncWithThread() const;
        MBOOL             triggerLoop() const;

    private:

        sp<ThreadHandle>               mpThread;
};


/*******************************************************************************
*
********************************************************************************/
class ICamThreadImpl::ThreadHandle : public Thread
{
    typedef enum ThreadStage{
        ThreadState_Idle       = 0x0,
        ThreadState_Init       = 0x1,
        ThreadState_UpdateLoop = 0x2,
    } ThreadStage;

    public:

                    ThreadHandle(
                            ICamThreadImpl* pNode,
                            eThreadNodeType const type,
                            MUINT32 policy, MUINT32 priority
                            );
                    ~ThreadHandle();

    private:

        status_t              readyToRun();
        bool                  threadLoop();
        MBOOL                 getCmd(ThreadCmdType* const cmd);
        MBOOL                 pushCmd(ThreadCmdType const cmd);

    public:

        MBOOL                 createThread();
        MBOOL                 sendThreadCmd(ThreadCmdType const cmd);
        MBOOL                 destroyThread();

        eThreadNodeType       getThreadNodeType() const { return mThreadType; }
        const char*           getName() const { return mpNode->getName(); }
        MINT32                getSensorIdx() const { return mpNode->getSensorIdx(); }

    private:

        class ThreadCmd : public virtual RefBase
        {
            public:
                ThreadCmdType mCmd     : 31 ;
            private:
                MBOOL         mPostSem : 1  ;
                sem_t         mSem          ;
            public:
                ThreadCmd(ThreadCmdType _cmd = TCmd_Sync, MBOOL _post = 0)
                    : mCmd(_cmd)
                      , mPostSem(_post)
                {
                    sem_init(&mSem, 0, 0);
                }

                ~ThreadCmd()
                {
                    sem_destroy(&mSem);
                }

                MBOOL postSem()
                {
                    if(mPostSem)
                        sem_post(&mSem);
                    return MTRUE;
                }

                MBOOL waitSem()
                {
                    if(mPostSem)
                    {
                        int ret;
                        while( 0 !=( ret = sem_wait( &mSem ) ) && errno == EINTR );
                        if( ret < 0 )
                        {
                            CAM_LOGE("%s errno = %d", __func__, errno);
                            //error log
                            return MFALSE;
                        }
                    }
                    return MTRUE;
                }
        };

        sem_t                          msemGetCmd;
        Mutex                          mThreadLock;
        ICamThreadImpl*                mpNode;
        eThreadNodeType const          mThreadType;
        MINT32 const                   mi4Policy;
        MINT32 const                   mi4Priority;
        ThreadStage                    mStage;
        ThreadCmdType                  mCurCmd;
        queue< sp<ThreadCmd> >         mqCmdQue;
};


/*******************************************************************************
*
********************************************************************************/
ICamNode::
ICamNode()
    : msName("ICamNode")
    , mpImpl(NULL)
{
    MY_LOGW("default ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamNode::
ICamNode(const char* name, ICamNodeImpl* const impl)
    : msName(name)
    , mpImpl( (!impl) ? new ICamNodeImpl(name, this) : impl )
{
    MY_LOGV("ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamNode::
~ICamNode()
{
    MY_LOGV("dtor");
    if( mpImpl )
        delete mpImpl;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNode::
setSensorIdx(MINT32 const idx)
{
    mpImpl->setSensorIdx(idx);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MINT32
ICamNode::
getSensorIdx() const
{
    return mpImpl->getSensorIdx();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNode::
addDataSupport(MBOOL const src_dst, MUINT32 const data)
{
    return mpImpl->addDataSupport(src_dst,data);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNode::
addNotifySupport(MUINT32 const msg)
{
    return mpImpl->addNotifySupport(msg);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNode::
isDataConnected(MUINT32 const data) const
{
    return mpImpl->isDataConnected(data);
}


/*******************************************************************************
*
********************************************************************************/
vector<MUINT32>
ICamNode::
getDataConnected() const
{
    return mpImpl->getDataConnected();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNode::
handlePostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext) const
{
    return mpImpl->handlePostBuffer(data, buf, ext);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNode::
handleReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext) const
{
    return mpImpl->handleReturnBuffer(data, buf, ext);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNode::
handleNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2) const
{
    return mpImpl->handleNotify(msg, ext1, ext2);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNode::
handleBroadcast(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2) const
{
    return mpImpl->handleBroadcast(msg, ext1, ext2);
}


/*******************************************************************************
*
********************************************************************************/
ICamBufHandler*
ICamNode::
getBufferHandler(MUINT32 const data) const
{
    return mpImpl->getBufferHandler(data);
}


/*******************************************************************************
*
********************************************************************************/
ICamGraph*
ICamGraph::
createInstance(MINT32 const sensorIdx, const char* usr)
{
    return new ICamGraph(sensorIdx, usr);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ICamGraph::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
ICamGraph::
ICamGraph()
    : msName("ICamGraph")
    , mpImpl(NULL)
{
    MY_LOGV("default ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamGraph::
ICamGraph(MINT32 const sensorIdx, const char* usr)
    : msName(usr)
    , mpImpl(new ICamGraphImpl(sensorIdx, usr, this) )
{
    MY_LOGV("ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamGraph::
~ICamGraph()
{
    MY_LOGV("dtor");
    if( mpImpl )
        delete mpImpl;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraph::
connectData(
        MUINT32 const updata,
        MUINT32 const downdata,
        ICamNode* const pUpNode,
        ICamNode* const pDownNode
        )
{
    return mpImpl->connectData(updata, downdata, pUpNode, pDownNode);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraph::
connectNotify(MUINT32 const msg, ICamNode* const pUpNode, ICamNode* const pDownNode)
{
    return mpImpl->connectNotify(msg, pUpNode, pDownNode);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraph::
disconnect()
{
    return mpImpl->disconnect();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraph::
init()
{
    return mpImpl->init();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraph::
uninit()
{
    return mpImpl->uninit();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraph::
start()
{
    return mpImpl->start();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraph::
stop()
{
    return mpImpl->stop();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraph::
broadcast(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    return mpImpl->broadcast(msg, ext1, ext2);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraph::
setBufferHandler(MUINT32 const data, ICamBufHandler* bufHdl, MINT32 idx)
{
    return mpImpl->setBufferHandler(data, bufHdl, idx);
}


/*******************************************************************************
*
********************************************************************************/
MINT32
ICamGraph::
getSensorIdx() const
{
    return mpImpl->getSensorIdx();
}


/*******************************************************************************
*
********************************************************************************/
ICamThreadNode::
ICamThreadNode()
    : ICamNode()
{
    MY_LOGV("default ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamThreadNode::
ICamThreadNode(const char* name, eThreadNodeType type, int policy, int priority)
    : ICamNode(name, new ICamThreadImpl(name, this, type, policy, priority))
{
    MY_LOGV("ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamThreadNode::
~ICamThreadNode()
{
    MY_LOGV("dtor");
}


/*******************************************************************************
*
********************************************************************************/
eThreadNodeType
ICamThreadNode::
getThreadNodeType() const
{
    return ((ICamThreadImpl*)mpImpl)->getThreadNodeType();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadNode::
syncWithThread() const
{
    return ((ICamThreadImpl*)mpImpl)->syncWithThread();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadNode::
triggerLoop() const
{
    return ((ICamThreadImpl*)mpImpl)->triggerLoop();
}


/*******************************************************************************
*
********************************************************************************/
ICamNodeImpl::
ICamNodeImpl()
    : mpSelf(NULL)
    , msName("NodeImpl")
    , miSensorIdx(-1)
    , mpGraphImpl(NULL)
    , mpBufMgr(NULL)
    , mState(State_Idle)
    , mbStopPostBuffer(MFALSE)
{
    //Mutex::Autolock _l(mLock);
    MY_LOGV("ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamNodeImpl::
ICamNodeImpl(const char* name, ICamNode* const pNode)
    : mpSelf(pNode)
    , msName(name)
    , miSensorIdx(-1)
    , mpGraphImpl(NULL)
    , mpBufMgr(NULL)
    , mState(State_Idle)
    , mbStopPostBuffer(MFALSE)
{
    //Mutex::Autolock _l(mLock);
    MY_LOGV("ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamNodeImpl::
~ICamNodeImpl()
{
    MY_LOGV("dtor");
    //Mutex::Autolock _l(mLock);
    if( !(mState == State_Idle) )
        MY_LOGE("dtor in state(0x%x)", mState);

    mvFlagSrc.clear();
    mvFlagDst.clear();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
connectData(
        MUINT32 const connect_type,
        ICamGraphImpl* const graph_impl,
        MUINT32 const srcdata,
        MUINT32 const dstdata,
        ICamNodeImpl* const pNode_impl)
{
    Mutex::Autolock _l(mLock);
    MBOOL ret = MFALSE;
    MY_ASSERT_STATE( mState < State_Initiated, mState );

    if( pNode_impl )
        MY_LOGV("connect data(%d), dstdata(%d), node(0x%x)", srcdata, dstdata, pNode_impl->mpSelf);

    if( !mpGraphImpl )
    {
        //first time connecting to the graph
        mpGraphImpl = graph_impl;
        //update sensor id
        setSensorIdx( mpGraphImpl->getSensorIdx() );
    }else{
        if(mpGraphImpl != graph_impl)
        {
            MY_LOGE("graph changed");
            goto lbExit;
        }
    }

    if( connect_type & NODE_CONNECT_AS_SRC )
    {
        if( !checkDataSupport(connect_type, srcdata) )
            goto lbExit;
    }
    else if( connect_type & NODE_CONNECT_AS_DST )
    {
        if( !checkDataSupport(connect_type, dstdata) )
            goto lbExit;
    }
    else if ( connect_type & NODE_CONNECT_AS_SINGLE )
    {
        ret = MTRUE;
        goto lbExit;
    }

    {
        list<DataLink>::const_iterator iter;
        if( connect_type & NODE_CONNECT_AS_SRC )
        {
            for( iter = mlDataLink.begin(); iter != mlDataLink.end(); iter++ )
            {
                if( iter->srcdata == srcdata )
                {
                    MY_LOGE("connect to same data %d, %d", iter->srcdata, srcdata );
                    goto lbExit;
                }
            }
            mvDataDst.push_back(srcdata);
        }
        else
        {
            for( iter = mlDataLink.begin(); iter != mlDataLink.end(); iter++ )
            {
                if( iter->dstdata == dstdata )
                {
                    MY_LOGE("connect to same data %d, %d", iter->dstdata, dstdata );
                    goto lbExit;
                }
            }
        }
        DataLink link = { srcdata, dstdata, pNode_impl };
        //MY_LOGD("add data(%d), dstdata(%d), size(%d)", srcdata, dstdata, mlDataLink.size());
        mlDataLink.push_back(link);
    }

    ret = MTRUE;
lbExit:
    mState = ret ? State_Connected : (State_Idle | State_Error);
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
connectNotify(
        ICamGraphImpl* const graph_impl,
        MUINT32 const msg,
        ICamNodeImpl* const pNode_impl)
{
    Mutex::Autolock _l(mLock);
    MBOOL ret = MFALSE;
    MY_ASSERT_STATE( mState < State_Initiated, mState );

    if( pNode_impl )
        MY_LOGV("connect msg(0x%x), node(0x%x)", msg, pNode_impl->mpSelf);

    if( !mpGraphImpl )
    {
        //first time connecting to the graph
        mpGraphImpl = graph_impl;
        //update sensor id
        setSensorIdx( mpGraphImpl->getSensorIdx() );
    }else{
        if(mpGraphImpl != graph_impl)
        {
            MY_LOGE("graph changed");
            goto lbExit;
        }
    }

    // if is the souce of notify
    if( pNode_impl )
    {
        if( !checkNotifySupport(msg) )
            goto lbExit;

        {
            list<NotifyLink>::iterator iter;
            for( iter = mlNotifyLink.begin(); iter != mlNotifyLink.end(); iter++ )
            {
                if( iter->mpNodeImpl == pNode_impl )
                {
                    list<MUINT32>::iterator iter_msg;
                    for( iter_msg = iter->mlMsg.begin(); iter_msg != iter->mlMsg.end(); iter_msg++ )
                    {
                        if( GET_NOTIFYCATEGORY(*iter_msg) == GET_NOTIFYCATEGORY(msg) )
                        {
                            *iter_msg |= msg;
                            ret = MTRUE;
                            goto lbExit;
                        }
                    }
                    iter->mlMsg.push_back(msg);
                    ret = MTRUE;
                    goto lbExit;
                }
            }
            NotifyLink link;
            link.mpNodeImpl = pNode_impl;
            link.mlMsg.push_back(msg);
            mlNotifyLink.push_back(link);
        }
    }

    ret = MTRUE;
lbExit:
    mState = ret ? State_Connected : (State_Idle | State_Error);
    return ret;

}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
disconnect()
{
    Mutex::Autolock _l(mLock);
    if( mState == State_Idle )
        return MTRUE;

    MY_ASSERT_STATE( mState & State_Connected, mState );

    MY_LOGV("disconnect");
    mvDataDst.clear();
    mlDataLink.clear();
    mlNotifyLink.clear();

    miSensorIdx = -1;
    mpGraphImpl = NULL;
    mpBufMgr    = NULL;

    mState = State_Idle;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
init()
{
    Mutex::Autolock _l(mLock);
    MY_ASSERT_STATE( mState == State_Connected, mState );

    MY_LOGV("init");
    MY_ASSERT( mpSelf->onInit() );

    mState = State_Initiated;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
uninit()
{
    Mutex::Autolock _l(mLock);
    MY_ASSERT_STATE( mState == State_Initiated, mState );

    MY_LOGV("uninit");
    MY_ASSERT( mpSelf->onUninit() );

    mState = State_Connected;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
start()
{
    Mutex::Autolock _l(mLock);
    MY_ASSERT_STATE( mState == State_Initiated, mState );

    MY_LOGV("Start");
    MY_ASSERT( mpSelf->onStart() );

    mState = State_Started;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
stop()
{
    Mutex::Autolock _l(mLock);
    MY_LOGV("Stop");
    if( mState <= State_Initiated )
    {
        MY_LOGD("already stopped");
        return MTRUE;
    }

    MY_ASSERT( mpSelf->onStop() );

    mState = State_Initiated;
    mbStopPostBuffer = MFALSE; //reset value
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
stopPostBuffer()
{
    //Mutex::Autolock _l(mLock); //no need
    // stop buffer flow
    mbStopPostBuffer = MTRUE;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL //FIXME: remove?
ICamNodeImpl::
Notify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_ASSERT( mpSelf->onNotify(msg, ext1, ext2) );
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
setCamBufMgr(CamBufMgr* bufmgr)
{
    Mutex::Autolock _l(mLock);
    mpBufMgr = bufmgr;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
ICamBufHandler*
ICamNodeImpl::
getBufferHandler(MUINT32 const data) const
{
    if( !mpBufMgr )
    {
        MY_LOGE("no buf mgr");
        return NULL;
    }

    return mpBufMgr->getBufferHandler(data);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
addDataSupport(MBOOL const src_dst, MUINT32 const data)
{
    Mutex::Autolock _l(mLock);
    MY_LOGV("addSupport data(%d)", data);

    vector<MUINT32> * pvToUpdate = (src_dst == ENDPOINT_SRC) ? &mvFlagSrc : &mvFlagDst;
    pvToUpdate->push_back(data);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
addNotifySupport(MUINT32 const msg)
{
    Mutex::Autolock _l(mLock);
    MY_LOGV("addSupport msg(0x%x)", msg);

    MUINT32 cat = GET_NOTIFYCATEGORY(msg);
    vector<MUINT32>::iterator iter;
    for( iter = mvNotify.begin(); iter != mvNotify.end() ; iter++ )
    {
        if( GET_NOTIFYCATEGORY(*iter) == cat )
        {
            *iter |= msg;
            return MTRUE;
        }
    }
    mvNotify.push_back(msg);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
isDataConnected(MUINT32 const data) const
{
    //Mutex::Autolock _l(mLock);
    MBOOL ret = MFALSE;
    list<DataLink>::const_iterator iter;
    for( iter = mlDataLink.begin(); iter != mlDataLink.end(); iter++ )
    {
        if( (iter->srcdata == data) || (iter->dstdata == data) )
        {
            ret = MTRUE;
            break;
        }
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
checkDataSupport(MUINT32 connect_type, MUINT32 data) const
{
    const vector<MUINT32> * pvFlag = (connect_type & NODE_CONNECT_AS_DST) ?
                                     &mvFlagSrc : &mvFlagDst;

    vector<MUINT32>::const_iterator iter;
    for( iter = pvFlag->begin() ; iter != pvFlag->end() ; iter++ )
    {
        if( *iter == data  )
            return MTRUE;
    }

    //dump supported
    MY_LOGE("connect type(0x%x), data(%d)", connect_type, data);
    MY_LOGW("=== dump supported start ===");
    for( iter = pvFlag->begin() ; iter != pvFlag->end() ; iter++ )
    {
        MY_LOGW("support: %d", *iter);
    }
    MY_LOGW("=== dump supported end ===");
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
checkNotifySupport(MUINT32 msg) const
{
    //FIXME

    vector<MUINT32>::const_iterator iter;
    for( iter = mvNotify.begin() ; iter != mvNotify.end() ; iter++ )
    {
        if( HAVE_NOTIFY( *iter, msg ) )
            return MTRUE;
    }

    //dump supported
    MY_LOGE("connect msg(0x%x)", msg);
    MY_LOGW("=== dump supported start ===");
    for( iter = mvNotify.begin() ; iter != mvNotify.end() ; iter++ )
    {
        MY_LOGW("support: 0x%x", *iter);
    }
    MY_LOGW("=== dump supported end ===");
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
handlePostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext) const
{
    //never use lock here
    ICamNodeImpl* nodeImpl = NULL;
    MUINT32 targetData = 0;
    const MBOOL bCheckDump = mpGraphImpl->isDumpEnable();

    // dump dst buffer
    if( bCheckDump && checkDumpData(data) )
    {
        mpSelf->onDumpBuffer(mpGraphImpl->getName(), data, buf, ext);
    }

    if( !getDataNode(NODE_CONNECT_AS_SRC, data, &nodeImpl, &targetData) )
    {
        MY_LOGV("data(%d) is not connected, return directly", data);
        return mpSelf->onReturnBuffer(data, buf, ext);
    }

    // dump src buffer
    if( bCheckDump && checkDumpData(targetData) )
    {
        nodeImpl->mpSelf->onDumpBuffer(mpGraphImpl->getName(), targetData, buf, ext);
    }

    MY_LOGV("post data(%d), buf(0x%x), ext(0x%x) -> data(%d) %s(0x%x)",
            data, buf, ext,
            targetData, nodeImpl->getName(), nodeImpl->mpSelf);
    return nodeImpl->mpSelf->onPostBuffer(targetData, buf, ext);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
handleReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext) const
{
    //never use lock here
    ICamNodeImpl* nodeImpl = 0;
    MUINT32 targetData = 0;
    if( !getDataNode(NODE_CONNECT_AS_DST, data, &nodeImpl, &targetData)  )
    {
        MY_LOGE("data(%d) is not connected, should not happen", data);
        dumpDataLink();
        return MFALSE;
    }

    MY_LOGV("return data(%d), buf(0x%x), ext(0x%x) -> data(%d) %s(0x%x)",
            data, buf, ext,
            targetData, nodeImpl->getName(), nodeImpl->mpSelf);
    return nodeImpl->mpSelf->onReturnBuffer(targetData, buf, ext);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
handleNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2) const
{
    //never use lock here
    MY_LOGV("handleNotify");
    MBOOL ret = MTRUE;
    list<NotifyLink>::const_iterator iter;
    for( iter = mlNotifyLink.begin(); iter != mlNotifyLink.end(); iter++ )
    {
        list<MUINT32>::const_iterator iter_msg;
        for( iter_msg = iter->mlMsg.begin(); iter_msg != iter->mlMsg.end(); iter_msg++ )
        {
            if( HAVE_NOTIFY(*iter_msg, msg) )
            {
                ICamNodeImpl* nodeImpl = iter->mpNodeImpl;
                ret = ret && nodeImpl->mpSelf->onNotify(msg, ext1, ext2);
                if( !ret )
                {
                    MY_LOGE("notify error at msg(0x%x), node(%s/0x%x)",
                            msg, nodeImpl->getName(), nodeImpl->mpSelf);
                }
                break;
            }
        }
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
handleBroadcast(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2) const
{
    //never use lock here
    MY_LOGV("handleBroadcast");
    if( mpGraphImpl )
    {
        return mpGraphImpl->broadcast(msg, ext1, ext2);
    }

    MY_LOGW("not connected with graph yet");
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
getDataNode(
        MUINT32 const type, MUINT32 const data,
        ICamNodeImpl** ppNode, MUINT32* pData
        ) const
{
    MBOOL ret = MFALSE;
    list<DataLink>::const_iterator iter;
#define FIND_NODE( comp_type, get_type, comp_data, pNodeImpl, targetdata ) \
    do{                                                                    \
        for( iter = mlDataLink.begin(); iter != mlDataLink.end(); iter++ ) \
        {                                                                  \
            if( iter->comp_type##data == comp_data )                       \
            {                                                              \
                (pNodeImpl)  = iter->nodeImpl;                             \
                (targetdata) = iter->get_type##data;                       \
                ret = MTRUE;                                               \
                break;                                                     \
            }                                                              \
        }                                                                  \
    }while(0)

    if( type & NODE_CONNECT_AS_SRC )
    {
        if( !mbStopPostBuffer ) {
            FIND_NODE(src, dst, data, *ppNode, *pData);
        }
    }
    else if( type & NODE_CONNECT_AS_DST )
    {
        FIND_NODE(dst, src, data, *ppNode, *pData);
    }
    //MY_LOGW("cannot find the targeted node, data(%d)", data);
#undef FIND_NODE
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ICamNodeImpl::
dumpDataLink() const
{
    list<DataLink>::const_iterator iter;
    for( iter = mlDataLink.begin(); iter != mlDataLink.end(); iter++ )
    {
        MY_LOGW("data(%d->%d), %s(0x%x)",
                iter->srcdata, iter->dstdata, iter->nodeImpl->getName(), iter->nodeImpl->mpSelf );
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamNodeImpl::
checkDumpData(MUINT32 const data) const
{
    MBOOL bDump = MFALSE;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    char key[PROPERTY_KEY_MAX];
    snprintf( key, PROPERTY_KEY_MAX, "debug.camnode.dump.%d", data);
    property_get( key, value, "0");
    if( (bDump = atoi(value)) )
    {
        MY_LOGD("enable dump buffer: %s, %d", getName(), data );
    }
    return bDump;
}


/*******************************************************************************
*
********************************************************************************/
CamBufMgr::
CamBufMgr(MINT32 const sensorIdx)
    : msName("CamBufMgr")
    , miSensorIdx(sensorIdx)
{
    MY_LOGD("ctor %d/%p", miSensorIdx, this );
}


/*******************************************************************************
*
********************************************************************************/
CamBufMgr::
~CamBufMgr()
{
    MY_LOGD("dtor %d/%p", miSensorIdx, this );
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamBufMgr::
reset()
{
    Mutex::Autolock _l(mLock);
    mvBufHdl.clear();
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
ICamBufHandler*
CamBufMgr::
getBufferHandler(MUINT32 const data) const
{
    Mutex::Autolock _l(mLock);
    vector< BufHdlMap >::const_iterator iter;
    for( iter = mvBufHdl.begin() ; iter != mvBufHdl.end() ; iter++ )
    {
        vector<MUINT32>::const_iterator data_iter;
        for( data_iter = iter->mvData.begin(); data_iter != iter->mvData.end(); data_iter++ )
        {
            if( *data_iter == data )
            {
                return iter->pHandle;
            }
        }
    }

#if 0
    MY_LOGE("cannot find buffer handler, data(0x%x), start dump", data);
    for( iter = mvBufHdl.begin() ; iter != mvBufHdl.end() ; iter++ )
    {
        MY_LOGW("data/handle: 0x%x/0x%x", iter->data, iter->pHandle);
    }
    MY_LOGE("cannot find buffer handler, end dump");
#endif
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamBufMgr::
setBufferHandler(MUINT32 const data, ICamBufHandler * const pBufHdl)
{
    Mutex::Autolock _l(mLock);
    vector< BufHdlMap >::iterator iter;
    for( iter = mvBufHdl.begin() ; iter != mvBufHdl.end() ; iter++ )
    {
        vector<MUINT32>::const_iterator data_iter;
        for( data_iter = iter->mvData.begin(); data_iter != iter->mvData.end(); data_iter++ )
        {
            if( *data_iter == data )
            {
                MY_LOGV("(0x%x) buffer handler already added: %d/0x%x", this, *data_iter, iter->pHandle);
                return MTRUE;
            }
        }
        // if same handler
        if( iter->pHandle == pBufHdl )
        {
            iter->mvData.push_back(data);
            MY_LOGD("(0x%x) update to same hdl: data(%d) hdl(0x%x)", this, data, iter->pHandle);
            return MTRUE;
        }
    }

    MY_LOGD("(0x%x) new bufhdl data(%d), hdl(0x%x)", this, data, pBufHdl);
    BufHdlMap newhdl;
    newhdl.pHandle = pBufHdl;
    newhdl.mvData.push_back(data);
    mvBufHdl.push_back(newhdl);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamBufMgr::
requestBuffer(MUINT32 const data, AllocInfo const & info) const
{
    ICamBufHandler* pBufHdl = getBufferHandler(data);
    return pBufHdl && pBufHdl->requestBuffer(data, info);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamBufMgr::
dequeBuffer(MUINT32 const data, ImgRequest * pImgReq) const
{
    ICamBufHandler* pBufHdl = getBufferHandler(data);
    return pBufHdl && pBufHdl->dequeBuffer(data, pImgReq);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamBufMgr::
enqueBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer) const
{
    ICamBufHandler* pBufHdl = getBufferHandler(data);
    return pBufHdl && pBufHdl->enqueBuffer(data, pImageBuffer);
}


/*******************************************************************************
*
********************************************************************************/
ICamGraphImpl::
ICamGraphImpl( MINT32 const sensorIdx, const char* name, ICamGraph* pGraph)
    : mpSelf(pGraph)
    , msName(name)
    , miSensorIdx(sensorIdx)
    , mState(State_Idle)
    , mbDumpEnable(MFALSE)
{
    MY_LOGV("ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamGraphImpl::
~ICamGraphImpl()
{
    vector< CamBufMgr* >::const_iterator iter;
    for( iter = mvpBufMgr.begin(); iter != mvpBufMgr.end(); iter++ )
    {
        delete *iter;
    }
    MY_LOGV("dtor");
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraphImpl::
connectData(
        MUINT32 const updata,
        MUINT32 const downdata,
        ICamNode* const pUpNode,
        ICamNode* const pDownNode
        )
{
    Mutex::Autolock _l(mLock);
    MY_ASSERT_STATE( mState < State_Initiated, mState );

    MBOOL ret = MFALSE;
    if( pUpNode && (pDownNode == NULL) && (updata == 0) && (downdata == 0) ) {
        //special case: single node
        ICamNodeImpl* up_impl   = pUpNode->mpImpl;
        if( !up_impl->connectData( NODE_CONNECT_AS_SINGLE, this, 0, 0, NULL) ) {
            goto lbExit;
        }
        addToNodeList(up_impl, NULL);
        ret = MTRUE;
        goto lbExit;
    }

    if( !(pUpNode && pDownNode) ) {
        MY_LOGE("up(0x%x), down(0x%x)", pUpNode, pDownNode );
        goto lbExit;
    }
    MY_LOGD("data(%d->%d), %s/%s (%d:0x%x/%d:0x%x)",
            updata, downdata,
            pUpNode->getName(), pDownNode->getName(),
            pUpNode->getSensorIdx(), pUpNode,
            pDownNode->getSensorIdx(), pDownNode);
    {
        ICamNodeImpl* up_impl   = pUpNode->mpImpl;
        ICamNodeImpl* down_impl = pDownNode->mpImpl;
        if( !up_impl->connectData( NODE_CONNECT_AS_SRC, this, updata, downdata, down_impl) ||
                !down_impl->connectData( NODE_CONNECT_AS_DST, this, updata, downdata, up_impl) )
        {
            goto lbExit;
        }

        addToNodeList(up_impl, down_impl);
    }
    ret = MTRUE;

lbExit:
    if( !ret )
        mState |= State_Error;

    if( ret )
        mState = State_Connected;

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraphImpl::
connectNotify(
        MUINT32 const msg,
        ICamNode* const pUpNode,
        ICamNode* const pDownNode
        )
{
    Mutex::Autolock _l(mLock);
    MY_ASSERT_STATE( mState < State_Initiated, mState );

    MBOOL ret = MFALSE;
    if( !(pUpNode && pDownNode) ) {
        MY_LOGE("up(0x%x), down(0x%x)", pUpNode, pDownNode );
        goto lbExit;
    }
    MY_LOGD("msg(0x%x), %s/%s (%d:0x%x/%d:0x%x)",
            msg,
            pUpNode->getName(), pDownNode->getName(),
            pUpNode->getSensorIdx(), pUpNode,
            pDownNode->getSensorIdx(), pDownNode);
    {
        ICamNodeImpl* up_impl   = pUpNode->mpImpl;
        ICamNodeImpl* down_impl = pDownNode->mpImpl;
        if( !up_impl->connectNotify( this, msg, down_impl) ||
                !down_impl->connectNotify( this, msg, NULL ) )
        {
            goto lbExit;
        }

        addToNodeList(up_impl, down_impl);
    }
    ret = MTRUE;

lbExit:
    if( !ret )
        mState |= State_Error;

    if( ret )
        mState = State_Connected;

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraphImpl::
disconnect()
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("disconnect");
    if( mState == State_Idle )
        return MTRUE;

    MY_ASSERT_STATE( mState & State_Connected, mState );

    MBOOL ret = MTRUE;
    vector< ICamNodeImpl* >::const_iterator iter;
    for( iter = mvNodeImpls.begin(); iter != mvNodeImpls.end(); iter++ )
    {
        MY_ASSERT( (*iter)->disconnect() );
    }

    mvNodeImpls.clear();

    vector< CamBufMgr* >::const_iterator iter_bufMgr;
    for( iter_bufMgr = mvpBufMgr.begin(); iter_bufMgr != mvpBufMgr.end(); iter_bufMgr++ )
    {
        (*iter_bufMgr)->reset();
    }

    if( !ret )
    {
        mState |= (State_Idle | State_Error);
    }
    else
    {
        mState = State_Idle | ( mState & (~State_Connected) );
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraphImpl::
init()
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("init +");
    MY_ASSERT_STATE( mState == State_Connected, mState );

    MBOOL ret = MTRUE;
    vector< ICamNodeImpl* >::const_iterator iter;
    for( iter = mvNodeImpls.begin(); iter != mvNodeImpls.end(); iter++ )
    {
        MY_ASSERT_NODE_OP( ret, (*iter), init );
    }

lbExit:
    if( !ret )
    {
        do {
            if( iter == mvNodeImpls.begin() )
                break;
            iter--;
            if( !(*iter)->uninit() )
                MY_LOGE("error handling: %s uninit fail", (*iter)->getName() );
        }while(1);
        mState |= State_Error;
    }
    else
    {
        mState = State_Initiated;
    }
    MY_LOGD("init -");
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraphImpl::
uninit()
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("uninit +");
    if( (mState&(~State_Error)) <= State_Connected )
    {
        MY_LOGD("already stopped");
        return MTRUE;
    }

    MY_ASSERT_STATE( mState & State_Initiated, mState );

    MBOOL ret = MTRUE;
    vector< ICamNodeImpl* >::const_iterator iter;
    for( iter = mvNodeImpls.begin(); iter != mvNodeImpls.end(); iter++ )
    {
        if( !(*iter)->uninit() )
        {
            MY_LOGE("%s uninit failed", (*iter)->getName());
            ret = MFALSE;
        }
    }

lbExit:
    if( !ret )
    {
        mState |= (State_Connected | State_Error);
    }
    else
    {
        mState = State_Connected | ( mState & (~State_Initiated) );
    }
    MY_LOGD("uninit -");
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraphImpl::
start()
{
    Mutex::Autolock _l(mLock);
    MY_LOGD("Start +");
    MY_ASSERT_STATE( mState == State_Initiated, mState );

    {   //update dump enable
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        char key[PROPERTY_KEY_MAX];
        snprintf( key, PROPERTY_KEY_MAX, "debug.%s.dump", getName());
        property_get( key, value, "0");
        if( (mbDumpEnable = atoi(value)) )
        {
            MY_LOGD("enable dump buffer: %s", getName() );
        }
    }

    MBOOL ret = MTRUE;
    vector< ICamNodeImpl* >::const_iterator iter = mvNodeImpls.end();
    while( iter != mvNodeImpls.begin() )
    {
        iter--;
        MY_ASSERT_NODE_OP( ret, (*iter), start );
    }

lbExit:
    if( !ret )
    {
        do {
            iter++;
            if( iter == mvNodeImpls.end() )
                break;
            if( !(*iter)->stop() )
                MY_LOGE("error handling: %s stop fail", (*iter)->getName() );
        }while(1);
        mState |= State_Error;
    }
    else
        mState = State_Started;

    MY_LOGD("Start -");
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraphImpl::
stop()
{
    Mutex::Autolock _l(mLock);
    MBOOL ret = MTRUE;
    MY_LOGD("Stop +");
    if( (mState&(~State_Error)) <= State_Initiated )
    {
        MY_LOGD("already stopped");
        return MTRUE;
    }

    MY_ASSERT_STATE( mState & State_Started, mState );


    vector< ICamNodeImpl* >::const_iterator iter;
    // stop post buffer first
    MY_LOGD("stop post buffer");
    for( iter = mvNodeImpls.begin(); iter != mvNodeImpls.end(); iter++ )
    {
        if( !(*iter)->stopPostBuffer() )
        {
            MY_LOGE("%s stopPostBuffer failed", (*iter)->getName());
            ret = MFALSE;
        }
    }

    // stop each
    for( iter = mvNodeImpls.begin(); iter != mvNodeImpls.end(); iter++ )
    {
        if( !(*iter)->stop() )
        {
            MY_LOGE("%s stop failed", (*iter)->getName());
            ret = MFALSE;
        }
    }

    // broadcast stopped
    if( !broadcast(LATE_STOP, 0, 0) )
        ret = MFALSE;

lbExit:
    if( !ret )
    {
        mState |= (State_Initiated | State_Error);
    }
    else
    {
        mState = State_Initiated | ( mState & (~State_Started) );
    }

    MY_LOGD("Stop -");
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraphImpl::
broadcast(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    //never use lock here
    MY_LOGV("broadcast msg(0x%x), ext(0x%x/0x%x)", msg, ext1, ext2);
    vector< ICamNodeImpl* >::const_iterator iter;
    for( iter = mvNodeImpls.begin(); iter != mvNodeImpls.end(); iter++ )
    {
        MY_ASSERT( (*iter)->Notify(msg, ext1, ext2) );
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraphImpl::
setBufferHandler(MUINT32 const data, ICamBufHandler* bufHdl, MINT32 idx)
{
    MBOOL ret = MFALSE;
    MBOOL hasCamBuf = MFALSE;
    MINT32 senorIdx = ( -1 == idx ) ? getSensorIdx() : idx;
    vector< CamBufMgr* >::const_iterator iter;
    for( iter = mvpBufMgr.begin(); iter != mvpBufMgr.end(); iter++ )
    {
        if( (*iter)->getSensorIdx() == senorIdx )
        {
            ret = (*iter)->setBufferHandler(data, bufHdl);
            hasCamBuf = MTRUE;
        }
    }
    if ( !hasCamBuf )
    {
        CamBufMgr* bufMgr = new CamBufMgr(senorIdx);
        ret = bufMgr->setBufferHandler(data, bufHdl);
        mvpBufMgr.push_back( bufMgr );
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamGraphImpl::
addToNodeList(ICamNodeImpl* up_impl, ICamNodeImpl* down_impl)
{
    MBOOL ret = MTRUE;
    MUINT32 flag = 0x3;
    vector< ICamNodeImpl* >::const_iterator iter;
    for( iter = mvNodeImpls.begin(); iter != mvNodeImpls.end(); iter++ )
    {
        if( (*iter) == up_impl )
        {
            flag &= ~(0x1);
        }else if ( (*iter) == down_impl) {
            flag &= ~(0x2);
        }
    }
    if( (flag & 0x1) && up_impl ) {
        up_impl->setSensorIdx( getSensorIdx() );
        mvNodeImpls.push_back( up_impl );
        vector< CamBufMgr* >::const_iterator iter;
        for( iter = mvpBufMgr.begin(); iter != mvpBufMgr.end(); iter++ )
        {
            if( (*iter)->getSensorIdx() == up_impl->getSensorIdx())
            {
                ret = ret && up_impl->setCamBufMgr(*iter);
                break;
            }
        }
    }
    if( (flag & 0x2) && down_impl ) {
        down_impl->setSensorIdx( getSensorIdx() );
        mvNodeImpls.push_back( down_impl );
        vector< CamBufMgr* >::const_iterator iter;
        for( iter = mvpBufMgr.begin(); iter != mvpBufMgr.end(); iter++ )
        {
            if( (*iter)->getSensorIdx() == down_impl->getSensorIdx())
            {
                ret = ret && down_impl->setCamBufMgr(*iter);
                break;
            }
        }
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
#if 0
MBOOL
ICamGraphImpl::
ReplaceNode(ICamNode* const pOld, ICamNode* const pNew)
{
    Mutex::Autolock _l(mLock);
    MY_LOGV("ReplaceNode old(0x%x), new(0x%x)", pOld, pNew);
#if 0
    if( mpGraphImpl )
    {
        return mpGraphImpl->broadcast(msg, ext1, ext2);
    }

    MY_LOGW("not connected with graph yet");
#endif
    return MFALSE;
}
#endif


/*******************************************************************************
*
********************************************************************************/
ICamThreadImpl::
ICamThreadImpl(
        const char* name,
        ICamThreadNode* const pNode,
        eThreadNodeType const type,
        MUINT32 policy, MUINT32 priority
        )
    : ICamNodeImpl(name, pNode)
    , mpThread(new ThreadHandle(this, type, policy, priority) )
{
    MY_LOGV("ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamThreadImpl::
~ICamThreadImpl()
{
    MY_LOGV("dtor");
    //TODO: check this
    mpThread = NULL;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::
init()
{
    Mutex::Autolock _l(mLock);
    MY_ASSERT_STATE( mState == State_Connected, mState );

    MY_LOGV("init");
    MY_ASSERT( mpSelf->onInit() );
    MY_ASSERT( mpThread->createThread()
            && mpThread->sendThreadCmd(TCmd_Sync)
            && mpThread->sendThreadCmd(TCmd_Init)
            && mpThread->sendThreadCmd(TCmd_Sync));

    mState = State_Initiated;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::
uninit()
{
    Mutex::Autolock _l(mLock);
    MY_ASSERT_STATE( mState == State_Initiated, mState );

    MY_LOGV("uninit");
    MY_ASSERT( mpThread->sendThreadCmd(TCmd_Uninit)
            && mpThread->sendThreadCmd(TCmd_Sync)
            && mpThread->destroyThread() );
    MY_ASSERT( mpSelf->onUninit() );

    mState = State_Connected;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::
start()
{
    Mutex::Autolock _l(ICamNodeImpl::mLock);
    MY_ASSERT_STATE( mState == State_Initiated, mState );

    MY_LOGV("Start");
    MY_ASSERT( mpSelf->onStart() );
    if( getThreadNodeType() == ContinuousTrigger ) {
        MY_ASSERT( mpThread->sendThreadCmd(TCmd_Update) );
    }

    mState = State_Started;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::
stop()
{
    Mutex::Autolock _l(ICamNodeImpl::mLock);
    MY_LOGV("Stop");
    if( mState <= State_Initiated )
    {
        MY_LOGD("already stopped");
        return MTRUE;
    }

    // need this? //TODO, FIXME: seems useful
    //MY_ASSERT( mpThread->sendThreadCmd(TCmd_Sync) );
    MY_ASSERT( mpSelf->onStop() );

    mState = State_Initiated;
    mbStopPostBuffer = MFALSE; //reset value
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
inline MBOOL
ICamThreadImpl::
handleThreadInit() const
{
    MBOOL ret = ((ICamThreadNode*)mpSelf)->threadInit();

    if( !ret )
    {
        MY_LOGE("%s:%s threadInit failed", mpGraphImpl->getName(), getName() );
    }
    return ret;
};


/*******************************************************************************
*
********************************************************************************/
inline MBOOL
ICamThreadImpl::
handleThreadUninit() const
{
    MBOOL ret = ((ICamThreadNode*)mpSelf)->threadUninit();

    if( !ret )
    {
        MY_LOGE("%s:%s threadUninit failed", mpGraphImpl->getName(), getName() );
    }
    return ret;
};


/*******************************************************************************
*
********************************************************************************/
inline MBOOL
ICamThreadImpl::
handleLoopUpdate() const
{
    MBOOL ret = ((ICamThreadNode*)mpSelf)->threadLoopUpdate();

    if( !ret )
    {
        MY_LOGE("%s:%s threadLoopUpdate failed", mpGraphImpl->getName(), getName() );
    }
    return ret;
};


/*******************************************************************************
*
********************************************************************************/
inline
eThreadNodeType
ICamThreadImpl::
getThreadNodeType() const
{
    return mpThread->getThreadNodeType();
};


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::
syncWithThread() const
{
    return mpThread->sendThreadCmd(TCmd_Sync);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::
triggerLoop() const
{
    return mpThread->sendThreadCmd(TCmd_Update);
}


/*******************************************************************************
*
********************************************************************************/
ICamThreadImpl::ThreadHandle::
ThreadHandle(ICamThreadImpl* pNode, eThreadNodeType const type, MUINT32 policy, MUINT32 priority)
    : mpNode(pNode)
    , mThreadType(type)
    , mi4Policy(policy)
    , mi4Priority(priority)
    , mStage(ThreadState_Idle)
    , mCurCmd(TCmd_Sync)
{
    sem_init(&msemGetCmd, 0, 0);
    MY_LOGV("ctor");
}


/*******************************************************************************
*
********************************************************************************/
ICamThreadImpl::ThreadHandle::
~ThreadHandle()
{
    MY_LOGV("dtor");
    // handle dtor in wrong state
    if( mStage != ThreadState_Idle )
        destroyThread();

    if( sem_destroy(&msemGetCmd) < 0 )
    {
        MY_LOGE("sem destroy failed)");
    }
}


/*******************************************************************************
*
********************************************************************************/
status_t
ICamThreadImpl::ThreadHandle::
readyToRun()
{
    MINT tid;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (mi4Policy == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, mi4Policy, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, mi4Priority);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = mi4Priority;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, mi4Policy, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), mi4Policy, mi4Priority);
    return NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
bool
ICamThreadImpl::ThreadHandle::
threadLoop()
{
    MY_LOGV("--loop--");
    bool nextloop = false;
    if( !getCmd(&mCurCmd) )
    {
        return false;
    }

    switch( mCurCmd )
    {
        case TCmd_Init:
            //FIXME call threadinit
            if( !mpNode->handleThreadInit() ) {
                nextloop = false;
            } else {
                mCurCmd = TCmd_Sync;
                nextloop = true;
            }
            break;
        case TCmd_Uninit: //FIXME
            if( !mpNode->handleThreadUninit() ) {
                nextloop = false;
            } else {
                mCurCmd = TCmd_Sync;
                nextloop = true;
            }
            break;
        case TCmd_Update:
            //loop event
            if( !mpNode->handleLoopUpdate() )
            {
                //modify cmd to none to block next loop,
                //leave threadstate ThreadState_UpdateLoop
                //mCurCmd = TCmd_Sync;
            }

            if ( mThreadType == SingleTrigger )
            {
                //single trigger, await next TCmd_Update
                mCurCmd = TCmd_Sync;
            }
            nextloop = true;
            break;
        case TCmd_Exit:
            mCurCmd = TCmd_Sync;
            nextloop = false;
            break;
        case TCmd_Sync:
            nextloop = true;
            break;
        default:
            MY_LOGD("not support cmd(0x%x)", mCurCmd );
            break;
    }
#undef ASSERT_STAGE

    if( !nextloop )
    {
        MY_LOGD("exit loop without requestExit");
    }

    //always next loop
    return nextloop;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::ThreadHandle::
getCmd(ThreadCmdType * const cmd)
{
    int ret;
    if( *cmd == TCmd_Sync )
    {
        MY_LOGV("wait for cmd");
        //ret = sem_wait( &msemGetCmd );
        while( 0 !=( ret = sem_wait( &msemGetCmd ) ) && errno == EINTR );
        if( ret < 0 )
        {
            MY_LOGE("errno = %d", errno);
            //error log
            return MFALSE;
        }
    }
    else
    {
        while( 0 !=( ret = sem_trywait( &msemGetCmd ) ) && errno == EINTR );
        if( ret < 0 )
        {
            if( errno == EAGAIN )
            {
                //no new cmd
                return MTRUE;
            }
            else
            {
                //error: EDEADLK, EINVAL
                MY_LOGE("errno = %d", errno);
                return MFALSE;
            }
        }
    }

    //get cmd
    sp<ThreadCmd> newCmd;
    {
        Mutex::Autolock _l(mThreadLock);
        newCmd = mqCmdQue.front();
        mqCmdQue.pop();
    }
    *cmd = newCmd->mCmd;
    MY_LOGV("cmd: 0x%x", *cmd);

    {  //just for debug
        //Mutex::Autolock _l(mThreadLock);
        switch(*cmd)
        {
#define ASSERT_STAGE( eCmd, exp, eNextstage )                                   \
            case eCmd:                                                          \
                  if( !(exp) )                                                  \
                  {                                                             \
                      MY_LOGW( "cmd %d: %s, stage(0x%x)", eCmd, #exp, mStage ); \
                      return MFALSE;                                            \
                  }                                                             \
                  mStage = eNextstage;                                          \
                break;

            //special case
            case TCmd_Sync: // no op
                if( mStage == ThreadState_UpdateLoop )
                    mStage = ThreadState_Init;
                break;
        ASSERT_STAGE( TCmd_Init   , mStage == ThreadState_Idle    , ThreadState_Init);
        ASSERT_STAGE( TCmd_Update , mStage >= ThreadState_Init    , ThreadState_UpdateLoop);
        ASSERT_STAGE( TCmd_Uninit , mStage == ThreadState_Init    , ThreadState_Idle);
        ASSERT_STAGE( TCmd_Exit   , mStage == ThreadState_Idle    , ThreadState_Idle);

        default:
            MY_LOGE("should not happen cmd(0x%x)", cmd);
            mStage = ThreadState_Idle;
            return MFALSE;
            break;
#undef ASSERT_STAGE
        }
    }

    newCmd->postSem();
    newCmd = NULL;

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::ThreadHandle::
pushCmd(ThreadCmdType const cmd)
{
    MBOOL wait;
    MY_LOGV("cmd 0x%x", cmd);
    {
        //Mutex::Autolock _l(mThreadLock);
        if( !isRunning() )
        {
            MY_LOGW("not running, skip cmd 0x%x", cmd);
            return MTRUE;
        }
    }

    wait = ::gettid() != getTid();
    sp<ThreadCmd> pNewCmd( new ThreadCmd(cmd, wait) );
    {
        Mutex::Autolock _l(mThreadLock);
        mqCmdQue.push(pNewCmd);
    }

    //notify new cmd
    sem_post(&msemGetCmd);

    if( !(cmd == TCmd_Update) ) //don't sync with update
    {
        pNewCmd->waitSem();
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::ThreadHandle::
createThread()
{
    switch( getThreadNodeType() )
    {
        case NO_THREAD:
            return MTRUE;
        case SingleTrigger:
        case ContinuousTrigger:
            return ( !isRunning() && OK == run(getName()) );
        default:
            MY_LOGE("not support thread type(%d)", mThreadType);
            break;
    }
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::ThreadHandle::
sendThreadCmd(ThreadCmdType const cmd)
{
    switch( getThreadNodeType() )
    {
        case NO_THREAD:
            return MTRUE;
        case SingleTrigger:
        case ContinuousTrigger:
            return isRunning() && pushCmd(cmd);
        default:
            MY_LOGE("not support thread type(%d)", mThreadType);
            break;
    }
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ICamThreadImpl::ThreadHandle::
destroyThread()
{
    switch( getThreadNodeType() )
    {
        case NO_THREAD:
            return MTRUE;
        case SingleTrigger:
        case ContinuousTrigger:
            return (!isRunning()) ? MTRUE : ( pushCmd(TCmd_Exit)    // request to exit
                                             && (join() == OK) );  // join
        default:
            MY_LOGE("not support thread type(%d)", mThreadType);
            break;
    }
    return MFALSE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

