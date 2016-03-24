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

#define LOG_TAG "Iop/P2PPWp.TD"

#if 0 //CHRISTOPHER, do not need in current stage
//
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include "HalPipeWrapper.h"
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/imageio/ispio_pipe_buffer.h>
//thread
#include <utils/threads.h>
#include <mtkcam/v1/config/PriorityDefs.h>
//thread priority
#include <system/thread_defs.h>
#include <sys/resource.h>
#include <utils/ThreadDefs.h>
// For property_get().
#include <cutils/properties.h>
//
/*************************************************************************************
* Log Utility
*************************************************************************************/
// Clear previous define, use our own define.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(P2PipeWrapper_Thread);
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (P2PipeWrapper_Thread_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (P2PipeWrapper_Thread_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (P2PipeWrapper_Thread_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (P2PipeWrapper_Thread_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (P2PipeWrapper_Thread_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (P2PipeWrapper_Thread_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)


using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;
extern PortIdxMappingP2toAll mPortIdxMappingP2toAll[EPostProcPortIdx_NUM];
extern nsecs_t nsTimeoutToWait;
extern MINT32 getUs();

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper::createThread()
{
    resetDequeVariables();
    pthread_create(&mThread, NULL, onThreadLoop, this);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper::destroyThread()
{
    // post exit means all user call uninit, clear all cmds in cmdlist and add uninit
    clearCommands(0);
    ECmd cmd=ECmd_UNINIT;
    addCommand(EPathCQ_CQ1,cmd);
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::resetDequeVariables()
{
    //clear and free mCmdList
    list<ECmd>().swap(mCmdList);
    mDequeueCQIdx=0;
    mTotalCmdNum=0;
    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
HalPipeWrapper::getDequeCQIdx()
{
    return mDequeueCQIdx;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::updateDequeCQIdx()
{
    //update dequeue cq index(cq0->cq1->cq2->cq0->...)
    mDequeueCQIdx=(mDequeueCQIdx+1) % EPathCQ_NUM;
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
#if 0
MBOOL
HalPipeWrapper::
sendCommand(
    int dequeCq,
    ECmd const cmd)
{
    LOG_DBG("+ dequeCq(%d) cmd(%d)",dequeCq,cmd);
    switch(cmd)
    {
        case ECmd_ENQUEUE:
            // signal to vss buffer if there is any incoming buffer (actually user should not enque any vss buffer if there still exist vss buffer in list)
            {    //the life cycle of mVssCondMtx is in the following region
                Mutex::Autolock autoLock(mVssCondMtx);
                mVssCond.signal();
            }
            // add enque cmd
            addCommand(dequeCq,ECmd_ENQUEUE);
            break;
        default:
            break;
    }
    return true;
}
#endif
/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper::
addCommand(
    int dequeCq,
    ECmd const &cmd)
{
    Mutex::Autolock autoLock(mDeQThreadMtx);

    //[1] add specific command to cmd list
    mCmdList.push_back(cmd);
    LOG_DBG("mCmdList, size(%d) + cmd(%d)", mCmdList.size(), cmd);
    mTotalCmdNum++;
    LOG_DBG("mTotalCmdNum_1(%d)",mTotalCmdNum);
    //[2] broadcast signal to user that a command is enqueued in cmd list
    mCmdListCond.broadcast();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper::
clearCommands(
    int dequeCq)
{
    Mutex::Autolock autoLock(mDeQThreadMtx);
    //[1] erase all the command in cmd list
    for (list<ECmd>::iterator it = mCmdList.begin(); it != mCmdList.end();)
    {
        //LOG_INF("cmd clear(%d)", *it);
        it = mCmdList.erase(it);
        it++;
    }
    mTotalCmdNum=0;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
getfirstCommand(
    int dequeCq,
    ECmd &cmd)
{
    Mutex::Autolock autoLock(mDeQThreadMtx);
    //[1] check if there is command in cmd list
    LOG_DBG("TEST mTotalCmdNum_2(%d)",mTotalCmdNum);
    if (mTotalCmdNum <= 0)
    {   //no more cmd in cq1/cq2/cq3 dequeueCmd list, block wait for user add cmd
        LOG_DBG("no more cmd, block wait");
        //always send a signal when all the enqueued buffer are dequeued to solve the problem we mention in doDequeue(pre)
        mpPostProcPipe->endequeFrameCtrl(NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_WAKE_WAITFRAME,0,0,0);
        mCmdListCond.wait(mDeQThreadMtx);   //wait for someone add enqueue cmd to list
    }
    //[2] get out the first command in correponding cmd list (first in first service)
    if(mCmdList.size()>0)
    {
        LOG_DBG("mCmdList curSize(%d), todo(%d)", mCmdList.size(), *mCmdList.begin());
        cmd = *mCmdList.begin();
        mCmdList.erase(mCmdList.begin());
        mTotalCmdNum--;
        //[3] condition that thread loop keep doing dequeue or not
        if (cmd == ECmd_UNINIT)
        {
            return MFALSE;
        }
        else
        {
            return MTRUE;
        }
    }
    else
    {
        cmd=ECmd_UNKNOWN;
        return MTRUE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
waitDequeVSSBufAgain()
{
    Mutex::Autolock autoLock(mVssCondMtx);
    //the condition timeout is used to aboid busy deque when there is only vss buffer in list
    //vss buffer is enqued to list if
    //(1) 3 ms later
    //(2) there exist an incoming prv buffer(blocking deque), cuz we could deque the buffer first if vss buffer deque is still not done
    mVssCond.waitRelative(mVssCondMtx, nsTimeoutToWait);

    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
dequeDoneNotify(
    EBufferListTag bufferListTag,
    MINT32 index,
    MINT32 p2dequeCQidx,
    MINT32 p2dupCQidx,
    MBOOL  isIPRaw,
    NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd cmd,
    MUINT32 callerID)
{
    LOG_DBG("+,tag(%d),idx(%d),p2CQ(%d)",bufferListTag,index,p2dequeCQidx);
    bool ret=true;

    if(bufferListTag==EBufferListTag_Batch)
    {
        getLock(ELockEnum_BufBatchEnQ);
        LOG_DBG("Lsize(%d)",mvEnqueuedBufBatch.size());
        vector<BufParamPackage>::iterator it =  mvEnqueuedBufBatch.begin();
        switch(index)
        {
            case 0:
                mvEnqueuedBufBatch.erase(it);
                break;
            case 1:
                it++;
                mvEnqueuedBufBatch.erase(it);
                break;
            default:
                LOG_ERR("Wrong element Idx!");
                break;
        }

        //update index for added buffer node in batchA
        LOG_DBG("updateBNList(%d)",mvEnqueueBufNode.size());
        for (list<BufParamNode>::iterator t = mvEnqueueBufNode.begin(); t != mvEnqueueBufNode.end(); t++)
        {
            if((*t).idxInBufBatchList>0)
            {
                (*t).idxInBufBatchList -= 1;
               }
        }
        //update real index for waited added buffer node in batchA
        LOG_DBG("updateBList(%d)",mvEnqueuedBufBatch.size());
        for(vector<BufParamPackage>::iterator t2 = mvEnqueuedBufBatch.begin(); t2 != mvEnqueuedBufBatch.end(); t2++)
        {
            if((*t2).idxINbatchLforBnode > 0)
            {
                (*t2).idxINbatchLforBnode -= 1;
               }
        }
        releaseLock(ELockEnum_BufBatchEnQ);

        getLock(ELockEnum_CQUser);
        if(p2dequeCQidx < 0)
        {
            releaseLock(ELockEnum_CQUser);
            LOG_ERR("wrong deque p2 cq (%d)",p2dequeCQidx);
            return MFALSE;
        }
        mp2CQUserNum[p2dequeCQidx]--;    //update enqueued user of corresponding cq
        releaseLock(ELockEnum_CQUser);

        //flush CQ descriptor if IP-RAW
        if(isIPRaw)
        {
            LOG_INF("IP_RAW flush (%d/%d)",p2dequeCQidx + EDrvCQ_CQ01,p2dupCQidx);
            mpPostProcPipe->sendCommand(0x1103,p2dequeCQidx + EDrvCQ_CQ01,0,p2dupCQidx);
        }

        //send signal to enqeue user that he could go ahead
        switch (p2dequeCQidx)
        {
            case EPathCQ_CQ1:
                LOG_DBG("Release CQ1 EnqueueBufList Resource");
                ::sem_post(&mSemCQ1);
                break;
            case EPathCQ_CQ2:
                LOG_DBG("Release CQ2 EnqueueBufList Resource");
                ::sem_post(&mSemCQ2);
                break;
            case EPathCQ_CQ3:
                LOG_DBG("Release CQ3 EnqueueBufList Resource");
                ::sem_post(&mSemCQ3);
                break;
            default:
                break;
       }
    }
    else
    {
        getLock(ELockEnum_BufNodeEnQ);
        list<BufParamNode>::iterator it =  mvEnqueueBufNode.begin();
        switch(index)
        {
            case 0:
                mvEnqueueBufNode.erase(it);
                break;
            case 1:
                it++;
                mvEnqueueBufNode.erase(it);
                break;
            default:
                LOG_ERR("Wrong element Idx!");
                break;
        }
        mpPostProcPipe->endequeFrameCtrl(cmd,callerID,0,p2dupCQidx);
        releaseLock(ELockEnum_BufNodeEnQ);
    }
    LOG_DBG("-");
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
dequeueJudgement()
{
    LOG_INF("+");
    //(pre) send signal to waiter to check dequeued buffer list
    //      * avoid the situation: dequeue finish signal of bufferA just coming right between user do condition check and go into wait when dequeue
    //      * always send a signal before dequeuing any buffer (if the problem mentioned above is met, do this before dequeuing bufferB could solve the problem)
    mpPostProcPipe->endequeFrameCtrl(NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_WAKE_WAITFRAME,0,0,0);
    MINT32 deqret=eIspRetStatus_Failed;
    bool ret=true;
    BufParamNode bufNode;
    MINT32 curWaitedDequeBufNodeNum=0;

    //[1] deque first element to judge
    getLock(ELockEnum_BufNodeEnQ);
    curWaitedDequeBufNodeNum=(MINT32)(mvEnqueueBufNode.size());
    list<BufParamNode>::iterator it = mvEnqueueBufNode.begin();
    bufNode=*it;
    releaseLock(ELockEnum_BufNodeEnQ);

    int dequeP2Cq=bufNode.p2cqIdx-EDrvCQ_CQ01;
    LOG_DBG("dequeCq(%d)/(%d)",dequeP2Cq,bufNode.p2cqIdx);
    switch(bufNode.bufTag)
    {
        case EBufferTag_Vss:
            //[2] do vss deque to check vss buffer is dequeued or not
            deqret=doDequeue(dequeP2Cq,bufNode,0);
            //TODO: only support 1 vss buffer at one time(no matter how many processes and threads)
            //[3] if vss buffer is not ready, do next buffer,
            if(deqret==eIspRetStatus_VSS_NotReady)
            {
                if(curWaitedDequeBufNodeNum==1)
                {    //only remaining vss buffer, add enque_cmd to do onThreadLoop and deque vss buffer again
                    waitDequeVSSBufAgain();
                    ECmd enqeueCmd=ECmd_ENQUEUE;
                    addCommand(2,enqeueCmd);
                    break;
                }
                else //second element must be blocking wait buffer
                {    //still have other blocking wait buffer
                    it++;
                    bufNode=(*it);
                    dequeP2Cq=bufNode.p2cqIdx-EDrvCQ_CQ01;
                    LOG_INF("dequeCq(%d)/(%d)",dequeP2Cq,bufNode.p2cqIdx);
                    if(bufNode.bufTag==EBufferTag_Vss)
                    {
                        LOG_ERR("More than two users use vss simultaneously");
                        return false;
                    }
                    deqret=doDequeue(dequeP2Cq,bufNode,1);
                    if(deqret==eIspRetStatus_Failed)
                    {
                        LOG_ERR("Dequeue Buffer Fail,cID(0x%x)!",bufNode.callerID);
                        ret=false;
                    }
                }
            }
            else if(deqret==eIspRetStatus_Failed)
            {    //vss buffer deque fail
                LOG_ERR("Dequeue Buffer Fail,cID(0x%x)!",bufNode.callerID);
                ret=false;
            }
            break;
        case EBufferTag_Blocking:
        default:
            //[2] wait for deque
            ret=mpPostProcPipe->endequeFrameCtrl(NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_WAIT_DEQUE,0,0,0);
            //[3] get signal and do deque
            if(ret==0)
            {
                LOG_ERR("Wait Deque Fail,cID(0x%x)!",bufNode.callerID);
                ret=false;
            }
            else
            {
                deqret=doDequeue(dequeP2Cq,bufNode,0);
                if(deqret==eIspRetStatus_Failed)
                {
                    LOG_ERR("Dequeue Buffer Fail,cID(0x%x)!",bufNode.callerID);
                    ret=false;
                }
            }
            break;
    }
    LOG_DBG("-");
    return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
MINT32
HalPipeWrapper::
doDequeue(
    MINT32 dequeP2Cq,
    BufParamNode bufNode,
    MINT32 elementIdx
)
{
    LOG_DBG("+");
    NSImageio_FrmB::NSIspio_FrmB::PortID portID;
    NSImageio_FrmB::NSIspio_FrmB::QTimeStampBufInfo rQTSBufInfo;
    NSImageio_FrmB::NSIspio_FrmB::BufInfo jpgBufInfo;
    rQTSBufInfo.vBufInfo.push_back(jpgBufInfo);
    MINT32 ret=eIspRetStatus_Success;
    MINT32 deQret=eIspRetStatus_Failed;
    MINT32 ioctrlret=0;
    MUINT32 jpgRealSize=0;
    MBOOL isVencContained=MFALSE;
    MBOOL isIPRaw=MFALSE;
    MUINT32 a=0,b=0;
    if(bufNode.portEn == ((1<<EPostProcPortIdx_IMGI) | 1<<EPostProcPortIdx_IMGO))
    {
        isIPRaw=MTRUE;
    }
    //[1] dequeue dma buffer
    for(int i = EPostProcPortIdx_VENC_STREAMO ; i > EPostProcPortIdx_VIP3I ; i--)
    {   //dequeue from wdma, wrot,jpeg,img2o,img3o,...
        if( bufNode.portEn & (1 << i))
        {
            portID.index=mPortIdxMappingP2toAll[i].ePortIndex;
            LOG_DBG("Dequeue out buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",portID.index,bufNode.p2cqIdx,bufNode.p2cqDupIdx);
            deQret=mpPostProcPipe->dequeOutBuf(portID,rQTSBufInfo,bufNode.drvSce,(MINT32)(bufNode.p2cqIdx),(MINT32)(bufNode.p2BurstQIdx),(MINT32)(bufNode.p2cqDupIdx));
            //
            switch(i)
            {
                case EPostProcPortIdx_VENC_STREAMO:
                    isVencContained=MTRUE;
                    break;
                default:
                    break;
            }
            //
            switch(deQret)
            {
                case eIspRetStatus_Failed:
                    LOG_ERR("dequeu Fail, out portID(%d),cq(%d),cqdupIdx(%d)",portID.index,(MINT32)(bufNode.p2cqIdx),(MINT32)(bufNode.p2cqDupIdx));
                    dequeDoneNotify(EBufferListTag_Node,elementIdx,dequeP2Cq,(MINT32)(bufNode.p2cqDupIdx),isIPRaw,NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_DEQUE_FAIL,bufNode.callerID);
                    //callback to notify deque fail
                    getLock(ELockEnum_BufBatchEnQ);
                    if(mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mvOut.size()>0)
                    {
                        LOG_INF("Fst mvOut mva(0x%x)",mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mvOut[0].mBuffer->getBufPA(0));
                    }
                    else
                    {
                        LOG_ERR("no output dma");
                    }
                    if(mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mpfnCallback != NULL)
                    {
                        mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mpfnCallback(mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams);
                    }
                    releaseLock(ELockEnum_BufBatchEnQ);
                    //
                    ::sem_post(&mSemVencCnt);
                    return deQret;
                case eIspRetStatus_VSS_NotReady:
                    return deQret;
                case eIspRetStatus_Success:
                default:
                    break;
            }

            //already get real size from deque output buffer function
            if(i==EPostProcPortIdx_JPEGO)
            {
                jpgRealSize= rQTSBufInfo.vBufInfo[0].jpg_size;
                //LOG_INF("[test]jpgRealSize(%d)",jpgRealSize);
            }
        }
    }
    for(int i = EPostProcPortIdx_VIP3I ; i > EPostProcPortIdx_NOSUP ; i--)
    {
        if( bufNode.portEn & (1 << i))
        {
            portID.index=mPortIdxMappingP2toAll[i].ePortIndex;
            LOG_DBG("Dequeue in buf portID(%d),p2cq(%d),p2cqDupIdx(%d)",portID.index,bufNode.p2cqIdx,bufNode.p2cqDupIdx);
            deQret=mpPostProcPipe->dequeInBuf(portID,rQTSBufInfo,bufNode.drvSce,(MINT32)(bufNode.p2cqIdx),(MINT32)(bufNode.p2BurstQIdx),(MINT32)(bufNode.p2cqDupIdx));
            switch(deQret)
            {
                case eIspRetStatus_Failed:
                    LOG_ERR("dequeu Fail, in portID(%d),cq(%d),cqdupIdx(%d)",portID.index,(MINT32)(bufNode.p2cqIdx),(MINT32)(bufNode.p2cqDupIdx));
                    dequeDoneNotify(EBufferListTag_Node,elementIdx,dequeP2Cq,(MINT32)(bufNode.p2cqDupIdx),isIPRaw,NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_DEQUE_FAIL,bufNode.callerID);
                    //callback to notify deque fail
                    getLock(ELockEnum_BufBatchEnQ);
                    if(mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mvOut.size()>0)
                    {
                        LOG_INF("Fst mvOut mva(0x%x)",mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mvOut[0].mBuffer->getBufPA(0));
                    }
                    else
                    {
                        LOG_ERR("no output dma");
                    }
                    if(mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mpfnCallback != NULL)
                    {
                        mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mpfnCallback(mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams);
                    }
                    releaseLock(ELockEnum_BufBatchEnQ);
                    return deQret;
                case eIspRetStatus_VSS_NotReady:
                    return deQret;
                case eIspRetStatus_Success:
                default:
                    break;
            }
        }
    }
    bool retFrame=mpPostProcPipe->dequeMdpFrameEnd(bufNode.drvSce);

    bool dequeSuccess=false;
    //[2] buffer node deque success
    if(retFrame==true)
    {
        //[2.1] add dequed count in buffer batch list
        getLock(ELockEnum_BufBatchEnQ);
        mvEnqueuedBufBatch[bufNode.idxInBufBatchList].dequedNum++;
        //for debug
        if(mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mvOut.size()>0)
        {
            LOG_INF("FmvOut(0x%x),dCq(%d/%d)",mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mvOut[0].mBuffer->getBufPA(0),bufNode.p2cqIdx,bufNode.p2cqDupIdx);
        }
        else
        {
            LOG_ERR("no output dma");
        }
        //judge
        if(mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mvMagicNo.size() == 0)
        {    //original version
            dequeSuccess=true;
        }
        else
        {    //slow motion support,//all buffer nodes in the batch are deque done
            if(mvEnqueuedBufBatch[bufNode.idxInBufBatchList].dequedNum == mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mvMagicNo.size())
            {dequeSuccess=true;}
        }

        //[2.2] do deque for user if all the buffer nodes in a buffer batch are dequed done
        if(dequeSuccess==true)
        {
            mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mDequeSuccess=MTRUE;
            //
            if(mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mpfnCallback == NULL)
            {
                //jpgo real size if needed
                if(bufNode.jpgo_idx>=0)
                {
                    LOG_INF("jpgRealSize(0x%x)",jpgRealSize);
                    mvEnqueuedBufBatch[bufNode.idxInBufBatchList].rParams.mvOut[bufNode.jpgo_idx].mBuffer->setBitstreamSize(jpgRealSize);    //TODO
                }
                BufParamPackage buf=mvEnqueuedBufBatch[bufNode.idxInBufBatchList];    //actually the first element
                releaseLock(ELockEnum_BufBatchEnQ);

                //add dequed buffer to dequed list before deque notify to make sure user's deque thread would get buffer frrom mvDequeuedBufBatch after being woken
                getLock(ELockEnum_BufBatchDeQ);
                mvDequeuedBufBatch.push_back(buf);
                LOG_INF("deque!dBLSize(%d)",(int)(mvDequeuedBufBatch.size()));
                releaseLock(ELockEnum_BufBatchDeQ);

                //pop out buffer node
                dequeDoneNotify(EBufferListTag_Node,elementIdx,dequeP2Cq,(MINT32)(bufNode.p2cqDupIdx),isIPRaw,NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_DEQUE_SUCCESS,bufNode.callerID);
                //deque notify for whole buffer batch
                dequeDoneNotify(EBufferListTag_Batch,elementIdx,dequeP2Cq,(MINT32)(bufNode.p2cqDupIdx),isIPRaw,NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_DEQUE_SUCCESS,bufNode.callerID);
            }
            else
            {
                LOG_DBG("idx(%d)",bufNode.idxInBufBatchList);
                BufParamPackage buf=mvEnqueuedBufBatch[bufNode.idxInBufBatchList];    //actually the first element
                releaseLock(ELockEnum_BufBatchEnQ);


                LOG_DBG("=go deque callback=");
                a=getUs();
                buf.rParams.mpfnCallback(buf.rParams);
                b=getUs();
                LOG_DBG("=callbackT(%d us)=",b-a);

                //pop out buffer node
                dequeDoneNotify(EBufferListTag_Node,elementIdx,dequeP2Cq,(MINT32)(bufNode.p2cqDupIdx),isIPRaw,NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_DEQUE_SUCCESS,bufNode.callerID);
                //deque notify for whole buffer batch
                dequeDoneNotify(EBufferListTag_Batch,elementIdx,dequeP2Cq,(MINT32)(bufNode.p2cqDupIdx),isIPRaw,NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_DEQUE_SUCCESS,bufNode.callerID);

                //clear buffer-batch record in kernel ourself cuz user do not call deuqe but we record info in kernel when enque, actually this should return immediately
                ioctrlret=mpPostProcPipe->endequeFrameCtrl(NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_WAIT_FRAME,bufNode.callerID,0, 0 , 1000000);
                if(!ioctrlret)
                {
                    LOG_ERR("clear kernel fail");
                    ret=eIspRetStatus_Failed;
                }
                else
                {
                    //if deque success(function return true), the return value should be changed to 0 which means eIspRetStatus_Success(0)
                    ret=eIspRetStatus_Success;
                }
            }
        }
        else
        {    // all the buffer node in batch are not ready
            releaseLock(ELockEnum_BufBatchEnQ);
            dequeDoneNotify(EBufferListTag_Node,elementIdx,dequeP2Cq,(MINT32)(bufNode.p2cqDupIdx),isIPRaw,NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_DEQUE_SUCCESS,bufNode.callerID);
        }
    }
    else
    {
        LOG_ERR("dequeMdpFrameEnd fail");
        dequeDoneNotify(EBufferListTag_Node,elementIdx,dequeP2Cq,(MINT32)(bufNode.p2cqDupIdx),isIPRaw,NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_DEQUE_FAIL,bufNode.callerID);
        return eIspRetStatus_Failed;
    }

    if(isVencContained)
    {
        getLock(ELockEnum_VencCnt);
        mVencPortCnt--;
        LOG_DBG("d mVencPortCnt(%d)",mVencPortCnt);
        releaseLock(ELockEnum_VencCnt);
        //notify
        ::sem_post(&mSemVencCnt);
    }
   LOG_INF("-,cb(%d us)",b-a);
   return ret;
}
/******************************************************************************
 *
 ******************************************************************************/
#define PR_SET_NAME 15
MVOID*
HalPipeWrapper::
onThreadLoop(
    MVOID *arg)
{
    //[1] set thread
    // set thread name
    ::prctl(PR_SET_NAME,"ISPDeQueueThread",0,0,0);
    // set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_PASS2;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if(policy == SCHED_OTHER)
    {    //  Note: "priority" is nice-value priority.
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, policy, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, priority);
    }
    else
    {    //  Note: "priority" is real-time priority.
        sched_p.sched_priority = priority;
        ::sched_setscheduler(0, policy, &sched_p);
    }
    //
    LOG_INF(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    //  detach thread => cannot be join, it means that thread would release resource after exit
    ::pthread_detach(::pthread_self());

    //[2] do dequeue buf if there is enqueue cmd in cmd list
    HalPipeWrapper *_this = reinterpret_cast<HalPipeWrapper*>(arg);
    ECmd cmd;
    bool ret=true;
    while(_this->getfirstCommand(_this->getDequeCQIdx(),cmd))
    {
        LOG_DBG("cmd(%d)",cmd);
        switch(cmd)
        {
            case ECmd_ENQUEUE:
                //someone enqueued, must do dequeue procedure here
                ret=_this->dequeueJudgement();
                break;
            case ECmd_UNINIT:
                goto EXIT;
                break;
            case ECmd_UNKNOWN:
            case ECmd_DEQUENEXTCQ:
            default:
                break;
        }
        cmd=ECmd_UNKNOWN;//prevent that condition wait in "getfirstCommand" get fake signal
    }
EXIT:
    _this->clearCommands(0);
    ::sem_post(&(_this->mSemDequeThread));
    LOG_INF("-");
    return NULL;
}

#endif

