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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_HALPIPEWRAPPER_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_HALPIPEWRAPPER_H_
//
//------------Thread-------------
#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <semaphore.h>
//-------------------------------
//
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <utils/threads.h>
#include <list>
#include <vector>
//
#include <mtkcam/iopipe/Port.h>
#include <mtkcam/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_scenario.h>
#include "isp_function_FrmB.h"

using namespace std;
using namespace android;


/*******************************************************************************
* real implement in imageio
********************************************************************************/
namespace NSImageio_FrmB{
namespace NSIspio_FrmB{
    class IPostProcPipe;
};
};

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc_FrmB {

#define MAX_ENQUEUE_USER_PERCQ  2
#define DEFAULT_P2_TUNING_QUE_BUFFER_NUM    5
/*******************************************************************************
* PostProc Pipe Port index.
********************************************************************************/
enum EPostProcPortIdx
{   //must in this order cuz deque order
    EPostProcPortIdx_NOSUP  =-1,
    EPostProcPortIdx_IMGI   = 0,      //0
    EPostProcPortIdx_IMGO   ,       //
    EPostProcPortIdx_IMG2O  ,
    EPostProcPortIdx_WROTO  ,       //
    EPostProcPortIdx_WDMAO  ,       //4
    //
    EPostProcPortIdx_NUM    = 5    //5
};

/*******************************************************************************
* Used CQ for enqueue/dequeue.
********************************************************************************/
enum EPathCQ
{
    EPathCQ_CQ1 = 0,
    EPathCQ_CQ2 = 1,
    EPathCQ_CQ3 = 2,
    //
    EPathCQ_NUM = 3
};

#define P2_CQ_DUPLICATION_MAX   2

/******************************************************************************
 *
 * @struct PortTypeMapping
 * @brief mapping relationship from cam port type to imageio port type.
 * @details
 *
 ******************************************************************************/
struct PortTypeMapping
{
    NSCam::NSIoPipe::EPortType      eCamPortType;
    NSImageio_FrmB::NSIspio_FrmB::EPortType   eImgIOPortType;
};

/******************************************************************************
 *
 * @struct PortIdxMappingP2toAll
 * @brief mapping relationship from p2 port index to all port index.
 * @details
 *
 ******************************************************************************/
struct PortIdxMappingP2toAll
{
    EPostProcPortIdx                eP2PortIdx;
    NSImageio_FrmB::NSIspio_FrmB::EPortIndex  ePortIndex;
};

/******************************************************************************
 *
 * @struct PortIdxMappingAlltoP2
 * @brief mapping relationship from all port index to p2 port index.
 * @details
 *
 ******************************************************************************/
struct PortIdxMappingAlltoP2
{
    NSImageio_FrmB::NSIspio_FrmB::EPortIndex  ePortIndex;
    EPostProcPortIdx                eP2PortIdx;
};

/******************************************************************************
 *
 * @struct TransformMapping
 * @brief mapping relationship about transformation(rotation and flip).
 * @details
 *
 ******************************************************************************/
struct TransformMapping
{
    MINT32                              eTransform; //eTransform in ImageFormat.h
    NSImageio_FrmB::NSIspio_FrmB::EImageRotation  eImgRot;
    NSImageio_FrmB::NSIspio_FrmB::EImageFlip      eImgFlip;   //TODO, need judge vflip/hflip
};

/******************************************************************************
 *
 * @struct SwHwScenarioPathMapping
 * @brief mapping relationship from sw scenario to hw path.
 * @details
 *
 ******************************************************************************/
struct SwHwScenarioPathMapping
{
    ESoftwareScenario               swScenario;
    const char*                     iHalPipeUser;
    NSImageio_FrmB::NSIspio_FrmB::EDrvScenario hwPath;
    NSImageio_FrmB::NSIspio_FrmB::EPipePass   ePipePass;
    EPathCQ                         ePathCQ;
};
/******************************************************************************
 *
 * @enum bufferTag
 * @brief mapping relationship from sw scenario to hw path.
 * @details
 *
 ******************************************************************************/

enum EBufferTag
{
    EBufferTag_Blocking=1,
    EBufferTag_Vss=2
};

/******************************************************************************
 *
 * @struct BufParamPackage
 * @brief parameter package for buffer batches enqueue/dequeue.
 * @details
 *
 ******************************************************************************/
struct BufParamPackage
{
    MUINT32 portEn;         //enable dmas
    NSIspDrv_FrmB::ISP_DRV_CQ_ENUM  p2cqIdx;        //p2 cq user use
    MINT32  p2cqDupIdx;     //p2 cq duplicate index(pinpon here)
    MINT32  p2BurstQIdx;
    MUINT32 callerID;       //different thread, special buffer id to recognize the buffer source(in default we use address of dequeuedBufList from caller)
    EBufferTag  bufTag;     //blocking or vss
    MINT32  jpgo_idx;       //index of jpego in mvOut vector
    QParams rParams;        //image buffer information
    MINT32  dequedNum;      //number of dequeued buffer
    NSImageio_FrmB::NSIspio_FrmB::EDrvScenario drvSce;
    MINT32  idxINbatchLforBnode;

    BufParamPackage(
        MUINT32     _portEn = 0x0,
        NSIspDrv_FrmB::ISP_DRV_CQ_ENUM      _p2cqIdx = NSIspDrv_FrmB::ISP_DRV_CQ01,
        MINT32      _p2cqDupIdx = 0,
        MUINT32     _callerID = 0,
        EBufferTag  _bufTag = EBufferTag_Blocking,
        MINT32      _jpgo_idx = -1)
    : portEn(_portEn)
    , p2cqIdx(_p2cqIdx)
    , p2cqDupIdx(_p2cqDupIdx)
    , callerID(_callerID)
    , bufTag(_bufTag)
    , jpgo_idx(_jpgo_idx)
    , drvSce(NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC)
    , dequedNum(0)
    , idxINbatchLforBnode(0)
    {}
};

/******************************************************************************
 *
 * @struct BufParamNode
 * @brief parameter node for buffer enqueue/dequeue.
 * @details
 *
 ******************************************************************************/
struct BufParamNode
{
    MUINT32 portEn;
    NSIspDrv_FrmB::ISP_DRV_CQ_ENUM  p2cqIdx;        //p2 cq user use
    MINT32  p2cqDupIdx;     //p2 cq duplicate index(pinpon here)
    MINT32  p2BurstQIdx;
    EBufferTag  bufTag;     //blocking or vss
    NSImageio_FrmB::NSIspio_FrmB::EDrvScenario drvSce;
    MUINT32 callerID;       //different thread, special buffer id to recognize the buffer source(in default we use address of dequeuedBufList from caller)
    MINT32  idxInBufBatchList;
    MINT32  jpgo_idx;       //index of jpego in mvOut vector
    BufParamNode()
    : portEn(0x0)
    , p2cqIdx(NSIspDrv_FrmB::ISP_DRV_CQ01)
    , p2cqDupIdx(0)
    , p2BurstQIdx(0)
    , bufTag(EBufferTag_Blocking)
    , drvSce(NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC)
    , callerID(0x0)
    , idxInBufBatchList(0)
    , jpgo_idx(-1)
    {}
};

/******************************************************************************
 *
 * @struct BufParamPackage
 * @brief parameter package for buffer enqueue/dequeue.
 * @details
 *
 ******************************************************************************/
struct EnqueuedRingBuf
{
    vector<BufParamPackage> bufferList;
    MINT32              userNum[EPathCQ_NUM];                         //current enqueued user num for each CQ
};

/******************************************************************************
 *
 * @enum ECmd
 * @brief enum for enqueue/dequeue signal control.
 * @details
 *
 ******************************************************************************/
enum ECmd
{
    ECmd_UNKNOWN = 0,
    ECmd_ENQUEUE,
    ECmd_DEQUENEXTCQ,
    ECmd_INIT,
    ECmd_UNINIT
};

/******************************************************************************
 *
 * @enum EBufferListTag
 * @brief enum for recognize buffer list type.
 * @details
 *
 ******************************************************************************/
enum EBufferListTag
{
    EBufferListTag_UNKNOWN = 0,
    EBufferListTag_Batch,
    EBufferListTag_Node
};

/******************************************************************************
 *
 * @enum ELockEnum
 * @brief enum for recognize lock type.
 * @details
 *
 ******************************************************************************/
enum ELockEnum
{
    ELockEnum_CQUser        = 0,
    ELockEnum_BufBatchEnQ      ,
    ELockEnum_BufBatchDeQ      ,
    ELockEnum_BufNodeEnQ       ,
    ELockEnum_VencCnt
};

/******************************************************************************
 *
 * @struct p2 tuninQueData
 * @brief parameter package for p2 tuningQuebuffer control
 * @details
 *
 ******************************************************************************/

struct tuninQueData {
    public:
        //void *pMem;
        IMEM_BUF_INFO bufferInfo;
    public:
        tuninQueData(
            void* _pMem = NULL
        )
        #if 0
        : pMem(_pMem)
        , bufferInfo(IMEM_BUF_INFO())
        #else
        : bufferInfo(IMEM_BUF_INFO())
        #endif
        {
        }
};



/******************************************************************************
 *
 * @class IPipeWrapper
 * @brief Wrapper for Normal Pipe Interface with PostProc Pipe.
 * @details
 * The path will be Normal Pipe --> PipeWraper --> PostProc Pipe.
 *
 ******************************************************************************/
class HalPipeWrapper
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Instantiation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    ////                    Constructor.
    HalPipeWrapper();
    ////                    Destructor.
    /**
     * @brief Disallowed to directly delete a raw pointer.
     */
    ~HalPipeWrapper() {};

public:     ////                    Instantiation.
    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static HalPipeWrapper*          createInstance();

    /**
     * @brief get the singleton instance
     *
     * @details
     *
     * @note
     *
     * @return
     * A singleton instance to this class.
     *
     */
    static HalPipeWrapper*          getInstance();

    /**
     * @brief destroy the pipe wrapper instance
     *
     * @details
     *
     * @note
     */
    MVOID                   destroyInstance();

    /**
     * @brief init the pipe wrapper
     *
     * @details
     *
     * @note
     *
     * @param[in] swScenario: sw scenario enum.
     * @param[in] callerPostProc: postproc pipe object for caller.
     * @param[in] isV3: use v3 flow or not.
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    MBOOL                   init(MINT32 swScenario,unsigned long& callerPostProc,MBOOL isV3, MUINT32 pixIdP2);
    /**
     * @brief uninit the pipe wrapper
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    MBOOL                   uninit(MINT32 swScenario,NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe *callerPostProc);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Attributes.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    /**
     * @brief get the last error code
     *
     * @details
     *
     * @note
     *
     * @return
     * - The last error code
     *
     */
    MERROR                  getLastErrorCode() const ;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Buffer Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    /**
     * @brief En-queue a request into the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] swScenario: sw scenario enum.
     * @param[in] rParams: Reference to a request of QParams structure.
     * @param[in] callerPostProc: postproc pipe object for caller.
     * @param[in] callerDeQList: dequeuedBufList of caller.
     * @param[in] bufferTag: buffer tag for indicating blocking wait or vss.
     *
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by \n
     *   getLastErrorCode().
     */
    MBOOL                  enque(MINT32 swScenario,
                                  QParams const& rParams,
                                  NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe *callerPostProc,
                                  vector<QParams>& callerDeQList,
                                  EBufferTag bufferTag);

    /**
     * @brief De-queue a result from the pipe.
     *
     * @details
     *
     * @note
     *
     * @param[in] rParams: Reference to a result of QParams structure.
     * @param[in] callerPostProc: postprocpipe object from caller.
     * @param[in] callerID: ID to recognize the specific buffer.
     * @param[in] i8TimeoutNs: timeout in nanoseconds \n
     *      If i8TimeoutNs > 0, a timeout is specified, and this call will \n
     *      be blocked until a result is ready. \n
     *      If i8TimeoutNs = 0, this call must return immediately no matter \n
     *      whether any buffer is ready or not. \n
     *      If i8TimeoutNs = -1, an infinite timeout is specified, and this call
     *      will block forever.
     * @param[in] addiStaData: Reference to a result of additional statistic data structure.
     *
     * @param[in] i4LastTime: last check time recorded in microseconds \n
     *
     * @return
     * - MTRUE indicates success;
     * - MFALSE indicates failure, and an error code can be retrived by getLastErrorCode().
     */
    MBOOL                  deque(QParams& rParams,
                                  NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe *callerPostProc,
                                  MUINT32 callerID,
                                  MINT64 i8TimeoutNs);
   /**
     * @brief notify start video record for slow motion support.
     *
     * @param[in] wd: width
     * @param[in] ht: height
     *
     * @details
     *
     * @note
     *
     * @return
     *      - [true]
     */
    MBOOL                    startVideoRecord(MINT32 wd,MINT32 ht, MINT32 fps,ESoftwareScenario swScen);
    /**
     * @brief notify stop video record for slow motion support.
     *
     * @details
     *
     * @note
     *
     * @return
     *      - [true]
     */
    MBOOL                    stopVideoRecord(ESoftwareScenario swScen);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  General Function.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   /**
     * @brief getLock.
     *
     * @details
     *
     * @note
     *
     * @return
     */
    MVOID   getLock(ELockEnum lockType);
    /**
     * @brief releaseLock.
     *
     * @details
     *
     * @note
     *
     * @return
     */
    MVOID   releaseLock(ELockEnum lockType);
    /**
     * @brief Check the current enqueued user number for a specific cq.
     *
     * @details
     *
     * @note
     *
     * @return
     *      - [true]   current enqueued user number meet the maximum number we support
     *      - [false]  current enqueued user number is less than the maximum number we support
     */
    MBOOL   checkCQEnqueuedUserNum(MINT32 p2cq);
    /**
     * @brief Query a specific buffer from deququed buf list.
     *
     * @details
     *
     * @note
     *
     * @param[in] params: the package(including frame number) we want to check whether is dequeued or not and return \n
     * @param[in] staParams: additional statistic info
     * @param[in] callerID: the caller id of the frame we want to query
     *
     * @return
     *      - [true]   get matched buffer
     *      - [false]  do not get matched buffer
     */
    MBOOL   queryFrame(QParams& params,MINT32 callerID);
    /**
         * @brief send isp extra command
         *
         * @details
         *
         * @note
         *
         * @param[in] cmd: command
         * @param[in] arg1: arg1
         * @param[in] arg2: arg2
         * @param[in] arg3: arg3
         *
         * @return
         * - MTRUE indicates success;
         * - MFALSE indicates failure, and an error code can be retrived by sendCommand().
         */
    MBOOL sendCommand(MINT32 cmd, MINTPTR arg1=0, MINTPTR arg2=0, MINTPTR arg3=0);
    /**
     * @brief get a buffer for tuning data provider
     *
     * @details
     *
     * @note
     *
     * @param[out] size: get the tuning buffer size
     * @param[out] pTuningQueBuf : get the tuning buffer point, user need to map to isp_reg_t(isp_reg.h) before use it.
     *
     * @return
     * - MTRUE  indicates success;
     * - MFALSE indicates failure;
     */
    MBOOL                   deTuningQue(unsigned int& size, void* &pTuningQueBuf);
    /**
     * @brief return the buffer which be got before
     *
     * @details
     *
     * @note
     *
     * @param[in] pTuningQueBuf : return the pTuningQueBuf
     *
     * @return
     * - MTRUE  indicates success;
     * - MFALSE indicates failure;
     */
    MBOOL                   enTuningQue(void* pTuningQueBuf);
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Thread Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    /**
     * @brief Create a thread dedicated for dequeue.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MVOID createThread();
    /**
     * @brief Destroy the thread dedicated for dequeue.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MVOID destroyThread();
     /**
     * @brief Add command to cmd list.
     *
     * @details
     *
     * @note
     *
     * @param[in] cmd: Command that we want to add to the cmd list.
     *
     * @return
     *
     */
    MVOID addCommand(int dequeCq,ECmd const &cmd);
     /**
     * @brief clear all commands in cmd list.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MVOID clearCommands(int dequeCq);
     /**
     * @brief get the first commands in cmd list.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MBOOL getfirstCommand(int dequeCq,ECmd &cmd);
     /**
     * @brief notify after deque no matter deque success or fail.
     *
     * @details
     * @param[in] bufferListTag: type of buffer list we want to pop out element.
     * @param[in] index: index of the element that we want to pop out.
     * @param[in] p2dequeCQidx: p2 cq index in that deque turn.
     * @param[in] p2dupCQidx: p2 duplicate cq index in that deque turn.
     * @param[in] cmd: deque success/fail.
     * @param[in] callerID: caller ID(enque-threadID).
     *
     * @note
     *
     * @return
     *
     */
   MBOOL dequeDoneNotify(EBufferListTag bufferListTag,
                                MINT32 index,
                                MINT32 p2dequeCQidx,
                                MINT32 p2dupCQidx,
                                MBOOL  isIPRaw,
                                NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd cmd=NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_DEQUE_SUCCESS,
                                MUINT32 callerID=0x0);
     /**
      * @brief judge buffer type.
      *
      * @details
      *
      * @note
      *
      * @return
      *
      */
     MBOOL dequeueJudgement();
     /**
     * @brief do dequeue procedure.
     *
     * @details
     *
      * @param[in] dequeCq: used p2 cq of buffer we wanna deque.
      * @param[in] bufPackage: buffer package we wanna deque.
      * @param[in] elementIdx: index of buffer package we wanna deque in EnqueuedBufList.
      *
     * @note
     *
     * @return
     *
     */
     MINT32 doDequeue(MINT32 dequeCq,BufParamNode bufNode,MINT32 elementIdx);
     /**
     * @brief Thread loop for dequeuing buffer.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    static  MVOID*  onThreadLoop(MVOID*);
     /**
     * @brief Reset all dequeue related variables in dequeue thread.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MBOOL resetDequeVariables();
    /**
     * @brief Get dequeue cq index.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MINT32 getDequeCQIdx();
     /**
     * @brief Update dequeue cq index.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MBOOL updateDequeCQIdx();
     /**
     * @brief wait condition do deque vss buffer again.
     *
     * @details
     *
     * @note
     *
     * @return
     *
     */
    MBOOL waitDequeVSSBufAgain();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //[general]
    mutable  Mutex      mLock;
    mutable  Mutex      mEnqueLock;
    mutable  Mutex      mTuningQueLock;
    volatile MINT32     mInitCount;
    list<const char*>   mUserNameList;
    sem_t               mSemCQ1;
    sem_t               mSemCQ2;
    sem_t               mSemCQ3;
    sem_t               mSemDequeThread;
    MINT32              mp2CQDupIdx[EPathCQ_NUM];
    MINT32              mp2CQUserNum[EPathCQ_NUM];  //current enqueued user num for each CQ
    //[enqueue]
    vector<BufParamPackage>     mvEnqueuedBufBatch;   //enqueued buffer batches list
    list<BufParamNode>  mvEnqueueBufNode;  //enqueued bufer node list
    //[dequeue thread]
    NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe       *mpPostProcPipe;
    MINT32              mDequeueCQIdx;
    pthread_t           mThread;
    list<ECmd>          mCmdList;
    MINT32              mTotalCmdNum;
    mutable  Mutex      mDeQThreadMtx;
    mutable  Condition  mCmdListCond;
    mutable  Mutex      mVssCondMtx;
    mutable  Condition  mVssCond;
    list<BufParamPackage> mvDequeuedBufBatch;
    //direct link to venc
    volatile MINT32     mVencPortCnt;
    sem_t               mSemVencCnt;
    //[tuning]
    MUINT32             mTuningQueNum;  // p2 tuning que number that be allocated
    vector<void*> gTuningQueNode;
    vector<tuninQueData> gTuningQueData;
    IMemDrv*            m_pIMemDrv ;
    MBOOL               misV3;
    MUINT32             mpixIdP2;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPostProc_FrmB
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_IOPIPE_POSTPROC_HALPIPEWRAPPER_H_

