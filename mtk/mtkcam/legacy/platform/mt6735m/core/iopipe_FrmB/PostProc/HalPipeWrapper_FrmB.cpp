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
#define LOG_TAG "Iop/P2PPWp_FrmB"

#include "HalPipeWrapper_FrmB.h"
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/ispio_sw_scenario.h>
#include <mtkcam/imageio/ispio_stddef.h>

#include <vector>

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <cutils/properties.h>
#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(P2PipeWrapper_FrmB);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (P2PipeWrapper_FrmB_DbgLogEnable_VERBOSE && MTKCAM_LOGENABLE_DEFAULT) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (P2PipeWrapper_FrmB_DbgLogEnable_DEBUG && MTKCAM_LOGENABLE_DEFAULT) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (P2PipeWrapper_FrmB_DbgLogEnable_INFO && MTKCAM_LOGENABLE_DEFAULT ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (P2PipeWrapper_FrmB_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (P2PipeWrapper_FrmB_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (P2PipeWrapper_FrmB_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc_FrmB;
using namespace NSImageio_FrmB;
using namespace NSIspio_FrmB;

pthread_mutex_t CQUserNumMutex = PTHREAD_MUTEX_INITIALIZER;    //mutex for cq user number when enqueue/dequeue
pthread_mutex_t BufBatchEnQMutex = PTHREAD_MUTEX_INITIALIZER;    //mutex for cq user number when enqueue/dequeue
pthread_mutex_t BufNodeEnQMutex = PTHREAD_MUTEX_INITIALIZER;    //mutex for biffer node number when enqueue/dequeue
pthread_mutex_t BufBatchDeQMutex = PTHREAD_MUTEX_INITIALIZER;  //mutex for dequeued buffer list read/write
nsecs_t nsTimeoutToWait = 3LL*1000LL*1000LL;//wait 3 msecs.


NSCam::NSIoPipe::NSPostProc_FrmB::PortTypeMapping mPortTypeMapping_FrmB[NSCam::NSIoPipe::EPortType_Memory+1] =
{
    {NSCam::NSIoPipe::EPortType_Sensor,NSImageio_FrmB::NSIspio_FrmB::EPortType_Sensor},
    {NSCam::NSIoPipe::EPortType_Memory,NSImageio_FrmB::NSIspio_FrmB::EPortType_Memory}
};

NSCam::NSIoPipe::NSPostProc_FrmB::SwHwScenarioPathMapping mSwHwPathMapping_FrmB[eSoftwareScenario_total_num]=
{
    {eSoftwareScenario_Main_Normal_Stream,   "Main_NormalStream",   NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC,  NSImageio_FrmB::NSIspio_FrmB::EPipePass_PASS2,  EPathCQ_CQ1},
    {eSoftwareScenario_Main_Normal_Capture,  "Main_NormalCapture",  NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC,  NSImageio_FrmB::NSIspio_FrmB::EPipePass_PASS2,  EPathCQ_CQ1},
    {eSoftwareScenario_Main_VSS_Capture,     "Main_VSSCapture",     NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_VSS, NSImageio_FrmB::NSIspio_FrmB::EPipePass_PASS2B, EPathCQ_CQ2},
    {eSoftwareScenario_Main_ZSD_Capture,     "Main_ZSDCapture",     NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC,  NSImageio_FrmB::NSIspio_FrmB::EPipePass_PASS2,  EPathCQ_CQ1},
    {eSoftwareScenario_Main_Pure_Raw_Stream, "Main_IPRawStream",    NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_IP,  NSImageio_FrmB::NSIspio_FrmB::EPipePass_PASS2,  EPathCQ_CQ1},
    //
    {eSoftwareScenario_Sub_Normal_Stream,    "Sub_NormalStream",    NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC,  NSImageio_FrmB::NSIspio_FrmB::EPipePass_PASS2,  EPathCQ_CQ1},
    {eSoftwareScenario_Sub_Normal_Capture,   "Sub_NormalCapture",   NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC,  NSImageio_FrmB::NSIspio_FrmB::EPipePass_PASS2,  EPathCQ_CQ1},
    {eSoftwareScenario_Sub_VSS_Capture,      "Sub_VSSCapture",      NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_VSS, NSImageio_FrmB::NSIspio_FrmB::EPipePass_PASS2B, EPathCQ_CQ2},
    {eSoftwareScenario_Sub_ZSD_Capture,      "Sub_ZSDCapture",      NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC,  NSImageio_FrmB::NSIspio_FrmB::EPipePass_PASS2,  EPathCQ_CQ1},
    {eSoftwareScenario_Sub_Pure_Raw_Stream,  "Sub_IPRawStream",     NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_IP,  NSImageio_FrmB::NSIspio_FrmB::EPipePass_PASS2,  EPathCQ_CQ1},
};

NSCam::NSIoPipe::NSPostProc_FrmB::PortIdxMappingAlltoP2 mPortIdxMappingAlltoP2_FrmB[NSImageio_FrmB::NSIspio_FrmB::EPortIndex_WDMAO+1]=
{
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_TG1I,   NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_NOSUP},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_TG2I,   NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_NOSUP},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMGI,   NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_IMGI},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_LSCI,   NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_NOSUP},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_CQI,    NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_NOSUP},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_TDRI,   NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_NOSUP},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMGO,   NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_IMGO},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMG2O,  NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_IMG2O},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_AAO,    NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_NOSUP},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_ESFKO,  NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_NOSUP},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_WROTO,  NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_WROTO},
    {NSImageio_FrmB::NSIspio_FrmB::EPortIndex_WDMAO,  NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_WDMAO},
};

NSCam::NSIoPipe::NSPostProc_FrmB::PortIdxMappingP2toAll mPortIdxMappingP2toAll_FrmB[NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_NUM]=
{
    {NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_IMGI,     NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMGI},
    {NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_IMGO,     NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMGO},
    {NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_IMG2O,    NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMG2O},
    {NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_WROTO,    NSImageio_FrmB::NSIspio_FrmB::EPortIndex_WROTO},
    {NSCam::NSIoPipe::NSPostProc_FrmB::EPostProcPortIdx_WDMAO,    NSImageio_FrmB::NSIspio_FrmB::EPortIndex_WDMAO}
};

#define TRANSFORM_MAX   8  //eTransform in ImageFormat.h
NSCam::NSIoPipe::NSPostProc_FrmB::TransformMapping mTransformMapping_FrmB[TRANSFORM_MAX]=
{
    {0,                 NSImageio_FrmB::NSIspio_FrmB::eImgRot_0,  NSImageio_FrmB::NSIspio_FrmB::eImgFlip_OFF},
    {eTransform_FLIP_H, NSImageio_FrmB::NSIspio_FrmB::eImgRot_0,  NSImageio_FrmB::NSIspio_FrmB::eImgFlip_ON},
    {eTransform_FLIP_V, NSImageio_FrmB::NSIspio_FrmB::eImgRot_0,  NSImageio_FrmB::NSIspio_FrmB::eImgFlip_ON},
    {eTransform_ROT_180,NSImageio_FrmB::NSIspio_FrmB::eImgRot_180,NSImageio_FrmB::NSIspio_FrmB::eImgFlip_OFF},
    {eTransform_ROT_90, NSImageio_FrmB::NSIspio_FrmB::eImgRot_90, NSImageio_FrmB::NSIspio_FrmB::eImgFlip_OFF},
    {0,                 NSImageio_FrmB::NSIspio_FrmB::eImgRot_0,  NSImageio_FrmB::NSIspio_FrmB::eImgFlip_OFF},
    {0,                 NSImageio_FrmB::NSIspio_FrmB::eImgRot_0,  NSImageio_FrmB::NSIspio_FrmB::eImgFlip_OFF},
    {eTransform_ROT_270,NSImageio_FrmB::NSIspio_FrmB::eImgRot_270,NSImageio_FrmB::NSIspio_FrmB::eImgFlip_OFF}
};



MINT32 getUs()
{
   struct timeval tv;
   ::gettimeofday(&tv, NULL);
   return tv.tv_sec * 1000000 + tv.tv_usec;
}

/******************************************************************************
 *
 ******************************************************************************/
HalPipeWrapper*
HalPipeWrapper::createInstance()
{
    return HalPipeWrapper::getInstance();
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HalPipeWrapper::destroyInstance()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
}

/******************************************************************************
 *
 ******************************************************************************/
static HalPipeWrapper Singleton;
HalPipeWrapper*
HalPipeWrapper::
getInstance()
{
    LOG_DBG("&Singleton(0x%x)\n",&Singleton);
    //
    return &Singleton;
}


/******************************************************************************
 *
 ******************************************************************************/
HalPipeWrapper::
HalPipeWrapper()
    : mpPostProcPipe(NULL)
    , mVencPortCnt(0)
    , mCmdListCond()
    , mVssCond()
{
    mp2CQDupIdx[0]=0;
    mp2CQDupIdx[1]=0;
    mp2CQDupIdx[2]=0;
    mp2CQUserNum[0]=0;
    mp2CQUserNum[1]=0;
    mp2CQUserNum[2]=0;
    misV3=false;
    DBG_LOG_CONFIG(imageio, P2PipeWrapper_FrmB);
    LOG_INF("dupidx CQ1/CQ2/CQ3 (%d/%d/%d)",mp2CQDupIdx[0],mp2CQDupIdx[1],mp2CQDupIdx[2]);
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
init(
    MINT32 swScenario,
    unsigned long& callerPostProc,
    MBOOL isV3,
    MUINT32 pixIdP2
)
{
    FUNCTION_LOG_START;
    bool ret = true;
    MUINT32 mP2TuningQueBufferSize = 0;
    // Automatic mutex. Declare one of these at the top of a function. It'll be locked when Autolock mutex is constructed and released when Autolock mutex goes out of scope.
    Mutex::Autolock lock(mLock);

    //[1] increase user count and record user
    android_atomic_inc(&mInitCount);
    LOG_INF("mUCnt(%d),mUser(%s)",mInitCount,mSwHwPathMapping_FrmB[swScenario].iHalPipeUser);
    if(strlen(mSwHwPathMapping_FrmB[swScenario].iHalPipeUser)<1)
    {
        LOG_ERR("Plz add userName\n");
        return MFALSE;
    }
    mUserNameList.push_back(mSwHwPathMapping_FrmB[swScenario].iHalPipeUser);

    //[2] map sw scenario to hw scenario
    //[3] create and init dedicated postproc pipe for halpipe user
    callerPostProc=(unsigned long)NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe::createInstance(mSwHwPathMapping_FrmB[swScenario].hwPath);
    if (NULL == callerPostProc || ! ((NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe *)callerPostProc)->init())
    {
        LOG_ERR("caller PostProc init fail");
        return false;
    }

    //first user
    if(mInitCount==1)
    {

        //temp workaround, flush CQ descriptor
        ((NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe *)callerPostProc)->sendCommand(0x1103,\
            mSwHwPathMapping_FrmB[swScenario].ePathCQ + NSIspDrv_FrmB::ISP_DRV_CQ01,0,0);
        ((NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe *)callerPostProc)->sendCommand(0x1103,\
            mSwHwPathMapping_FrmB[swScenario].ePathCQ + NSIspDrv_FrmB::ISP_DRV_CQ01,0,1);
        //

        //
        misV3=isV3;
        mpixIdP2 = pixIdP2;

        //[4]initialize variables
        for(int i=0;i<EPathCQ_NUM;i++)
        {
            mp2CQDupIdx[i]=0;
            mp2CQUserNum[i]=0;
        }
        sem_init(&mSemCQ1, 0, 0);
        sem_init(&mSemCQ2, 0, 0);
        sem_init(&mSemCQ3, 0, 0);
        sem_init(&mSemDequeThread, 0, 0);
        sem_init(&mSemVencCnt, 0, 0);
        for (list<BufParamPackage>::iterator it = mvDequeuedBufBatch.begin(); it != mvDequeuedBufBatch.end();)
        {
            //LOG_INF("cmd clear(%d)", *it);
            it = mvDequeuedBufBatch.erase(it);
            it++;
        }

        #if !defined(__PMEM_ONLY__)  //imem
    // create imem driver
    m_pIMemDrv = IMemDrv::createInstance();
    LOG_DBG("[m_pIMemDrv]:0x%08x ",m_pIMemDrv);
    if ( NULL == m_pIMemDrv ) {
        LOG_ERR("IMemDrv::createInstance fail.");
        return -1;
    }
    m_pIMemDrv->init();
        #endif //!defined(__PMEM_ONLY__)

        //
        // initialize p2 tuningQue buffer
        mP2TuningQueBufferSize = sizeof(isp_reg_t);
        if(!gTuningQueNode.empty() || !gTuningQueData.empty()) {
            LOG_ERR("[Error]gTuningQueNode|gTuningQueData buffer is not empty!");
            return -1;
        } else {
            tuninQueData tuningQueDataNew[DEFAULT_P2_TUNING_QUE_BUFFER_NUM];
            //
            mTuningQueNum = DEFAULT_P2_TUNING_QUE_BUFFER_NUM;
            for(int i=0;i<DEFAULT_P2_TUNING_QUE_BUFFER_NUM;i++) {
                #if defined(__PMEM_ONLY__)
                    tuningQueDataNew[i].bufferInfo.virtAddr = (MUINT32*)pmem_alloc_sync(size, &gTuningQueData[i].bufferInfo.memID);
                    memset((MUINT8*)tuningQueDataNew[i].bufferInfo.virtAddr,0,size);
                #else
                    tuningQueDataNew[i].bufferInfo.size = mP2TuningQueBufferSize;
                    tuningQueDataNew[i].bufferInfo.useNoncache = 0; //alloc cacheable mem.
                    if ( m_pIMemDrv->allocVirtBuf(&tuningQueDataNew[i].bufferInfo) ) {
                        LOG_ERR("[ERROR]:m_pIMemDrv->allocVirtBuf");
                    }
                    memset((MUINT8*)tuningQueDataNew[i].bufferInfo.virtAddr,0,mP2TuningQueBufferSize);
                #endif
                LOG_INF("[TuningQue]i(%d),VA(0x%x),size(0x%x)\n",i,tuningQueDataNew[i].bufferInfo.virtAddr,mP2TuningQueBufferSize);
                gTuningQueData.push_back(tuningQueDataNew[i]);
                gTuningQueNode.push_back((void*)tuningQueDataNew[i].bufferInfo.virtAddr);
            }
        }

        //[5] create postproc pipe dedicated for dequeue buf
        mpPostProcPipe=NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe::createInstance(NSImageio_FrmB::NSIspio_FrmB::eDrvScenario_CC);//no matter
        if (NULL == mpPostProcPipe || !mpPostProcPipe->init())
        {
            LOG_ERR("dequeue PostProc init fail");
            return false;
        }
        //[6] create dequeue thread
        createThread();
    }
    else
    {    //check version if more than 1 users
        if(misV3 != isV3)
        {
            LOG_ERR("wrong version mV3/isV3 (%d/%d)",misV3,isV3);
        }
    }

    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
uninit(
    MINT32 swScenario,
    NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe * callerPostProc)
{
    FUNCTION_LOG_START;
    bool ret = true;
    Mutex::Autolock lock(mLock);
    //[1] decrease user count and record user
    android_atomic_dec(&mInitCount);
    LOG_INF("mUCnt(%d),mUser(%s)",mInitCount,mSwHwPathMapping_FrmB[swScenario].iHalPipeUser);
    if(strlen(mSwHwPathMapping_FrmB[swScenario].iHalPipeUser)==0)
    {
    }
    else
    {
        #if 0   //temp remove, need check
        mUserNameList.remove(mSwHwPathMapping_FrmB[swScenario].iHalPipeUser);
        list<const char*>::iterator iter;
        char* userlist=(char*)malloc(128*sizeof(char));
        for (iter = mUserNameList.begin(); iter != mUserNameList.end(); ++iter)
        {
           strcat(userlist,*iter);
           strcat(userlist,",");
        }
        LOG_INF("HalPipeWrapper RestUsers:%s\n",userlist);
        free(userlist);
        #endif
    }

    //deq thread related variables are init only once
    if(mInitCount==0)
    {

        //[2] stop dequeue thread
        destroyThread();
        ::sem_wait(&mSemDequeThread);

        //[3] create postproc pipe dedicated for dequeue buf
        if (true != mpPostProcPipe->uninit())
        {
            LOG_ERR("PostProc uninit fail");
            ret = false;
        }

        mpPostProcPipe->destroyInstance();
        mpPostProcPipe = NULL;

        //[4]free vector variable
        vector<BufParamPackage>().swap(mvEnqueuedBufBatch); //force vector to relinquish its memory (look up "stl swap trick")
        // free p2 tuning Queue
        LOG_DBG("Free Tuning Queue");

        // check tuning que size
        if(mTuningQueNum != gTuningQueData.size()){
            LOG_ERR("[Error]gTuningQueData number(%d) is different from that be alloced(%d)",
                mTuningQueNum,gTuningQueData.size());
            return -1;
        }
        if(mTuningQueNum != gTuningQueNode.size()){
            LOG_ERR("[Error]gTuningQueNode number(%d) is different from that be alloced(%d)",
                mTuningQueNum,gTuningQueNode.size());
            return -1;
        }

        for(int i=0;i<mTuningQueNum;i++) {
            if((gTuningQueData[i].bufferInfo.memID>=0) && (gTuningQueData[i].bufferInfo.virtAddr!=0)){
                #if defined(__PMEM_ONLY__)
                    pmem_free((MUINT8*)gTuningQueData[i].bufferInfo.virtAddr,gTuningQueData[i].bufferInfo.size,gTuningQueData[i].bufferInfo.memID);
                    gTuningQueData[i].bufferInfo.virtAddr = NULL;
                    gTuningQueData[i].bufferInfo.memID = -1;
                    gTuningQueNode[i] = NULL;
                #else
                    if ( m_pIMemDrv->freeVirtBuf(&gTuningQueData[i].bufferInfo) ) {
                        LOG_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
                    }
                    gTuningQueData[i].bufferInfo.virtAddr = NULL;
                    gTuningQueData[i].bufferInfo.memID = -1;
                    gTuningQueNode[i] = NULL;
                #endif
            } else {
                LOG_ERR("[Error]free TuningQue error i(%d),fd(%d),va(0x%08x)",i,gTuningQueData[i].bufferInfo.memID,gTuningQueData[i].bufferInfo.virtAddr);
            }
        }
        gTuningQueData.erase(gTuningQueData.begin(),gTuningQueData.end());
        gTuningQueNode.erase(gTuningQueNode.begin(),gTuningQueNode.end());
        LOG_DBG("[TuningQueSize]%d-%d",gTuningQueData.size(),gTuningQueNode.size());

        //destroy imem
        #if !defined(__PMEM_ONLY__)  //imem
        m_pIMemDrv->uninit();
        LOG_DBG("m_pIMemDrv->uninit()");
        m_pIMemDrv->destroyInstance();
        LOG_DBG("m_pIMemDrv->destroyInstance()");
        m_pIMemDrv = NULL;
        #endif  //!defined(__PMEM_ONLY__)

    }

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
*
******************************************************************************/
MVOID
HalPipeWrapper::
getLock(
    ELockEnum lockType)
{
    switch(lockType)
    {
        case ELockEnum_CQUser:
            pthread_mutex_lock(&CQUserNumMutex);
            break;
        case ELockEnum_BufBatchEnQ:
            pthread_mutex_lock(&BufBatchEnQMutex);
            break;
        case ELockEnum_BufBatchDeQ:
            pthread_mutex_lock(&BufBatchDeQMutex);
            break;
        case ELockEnum_BufNodeEnQ:
            pthread_mutex_lock(&BufNodeEnQMutex);
            break;
        default:
            break;
    }
}

/******************************************************************************
*
******************************************************************************/
MVOID
HalPipeWrapper::
releaseLock(
    ELockEnum lockType)
{
    switch(lockType)
    {
        case ELockEnum_CQUser:
            pthread_mutex_unlock(&CQUserNumMutex);
            break;
        case ELockEnum_BufBatchEnQ:
            pthread_mutex_unlock(&BufBatchEnQMutex);
            break;
        case ELockEnum_BufBatchDeQ:
            pthread_mutex_unlock(&BufBatchDeQMutex);
            break;
        case ELockEnum_BufNodeEnQ:
            pthread_mutex_unlock(&BufNodeEnQMutex);
            break;
        default:
            break;
    }
}


/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
checkCQEnqueuedUserNum(
    MINT32 p2cq)
{
    getLock(ELockEnum_CQUser);
    if(mp2CQUserNum[p2cq]==MAX_ENQUEUE_USER_PERCQ)
    {
        releaseLock(ELockEnum_CQUser);
        return true;
    }
    else
    {
        releaseLock(ELockEnum_CQUser);
        return false;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
enque(
    MINT32 swScenario,
    QParams const& rParams,
    NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe * callerPostProc,
    vector<QParams>& callerDeQList,
    EBufferTag bufferTag)
{

    //FUNCTION_LOG_START;
    Mutex::Autolock lock(mEnqueLock);
    bool ret = false;
    BufParamPackage bufPackage;
    int pathCQ=mSwHwPathMapping_FrmB[swScenario].ePathCQ;

    //debug
    //getLock(ELockEnum_CQUser);
    //LOG_INF("pathCQ(%d), curUsrNum(%d)",pathCQ,mp2CQUserNum[pathCQ]);
    //releaseLock(ELockEnum_CQUser);

    //[1] block if meet the max enqueue user for the CQ)
    while(checkCQEnqueuedUserNum(pathCQ))
    {
        switch (pathCQ)
        {
            case EPathCQ_CQ1:
                LOG_INF("Block CQ1 Enqueue Here");
                ::sem_wait(&mSemCQ1);
                break;
            case EPathCQ_CQ2:
                LOG_INF("Block CQ2 Enqueue Here");
                ::sem_wait(&mSemCQ2);
                break;
            case EPathCQ_CQ3:
                LOG_INF("Block CQ3 Enqueue Here");
                ::sem_wait(&mSemCQ3);
                break;
            default:
                break;
        }
    }

    //[2] using cq and duplicate index
    switch(pathCQ)
    {
        case EPathCQ_CQ1:
            bufPackage.p2cqIdx=NSIspDrv_FrmB::ISP_DRV_CQ01;
            break;
        case EPathCQ_CQ2:
            bufPackage.p2cqIdx=NSIspDrv_FrmB::ISP_DRV_CQ02;
            break;
        case EPathCQ_CQ3:
        default:
            LOG_ERR("no support this p2 path(%d)",pathCQ);
            break;
    }
    getLock(ELockEnum_CQUser);
    bufPackage.p2cqDupIdx=mp2CQDupIdx[pathCQ];
    releaseLock(ELockEnum_CQUser);

    int pixelInByte=1;

    //judge buffer package number in a enque request
    int bufPackageNum=0;
    if(misV3)
    {
        bufPackageNum=rParams.mpTuningData.size();
    }
    else
    {
        bufPackageNum=rParams.mvMagicNo.size();
    }
    LOG_DBG("bufPackageNum(%d),misV3(%d),magic(%d),tuningData(%d)",bufPackageNum,misV3,rParams.mvMagicNo.size(),rParams.mpTuningData.size());


    if(bufPackageNum==0)
    {    //original version
       LOG_ERR("DO NOT SUPPORT THIS, PLZ USE VECTOR TYPE");
    }
    else
    {

        //slow motion, burst Q version
        //[3]judge that the number of enque buffer batches exceeds supported number or not

            //[4]parse the seperately number of in/out ports in each buffer
            MINT32 planeNum=0;
        EPostProcPortIdx p2PortIdx=EPostProcPortIdx_NOSUP;
        int inPorts=0,outPorts=0;
        MINT32 inStartIdx=0, outStartIdx=0;
        MINT32 curgroup=0;
        MINT32 inNum[bufPackageNum];        //input dma num for each buffer node
        MINT32 outNum[bufPackageNum];    //output dma num for each buffer node
        int tmpC=0;
        int inCnt=0,outCnt=0;
            for(int q=0;q<bufPackageNum;q++)
        {
            //mvin
            tmpC=0;
            for(int i=inStartIdx;i< rParams.mvIn.size() ;i++)
                {
                    //check cur element is next buffer group or not
                    if(tmpC==0)
                    {    //first element in this buffer group
                        curgroup=rParams.mvIn[i].mPortID.group;
                        inNum[q]=0;
                    }
                if(rParams.mvIn[i].mPortID.group != curgroup)
                {
                    inStartIdx=i;//set startIdx for next run
                    break;
                }
                //port num
                inNum[q]++;
                inPorts+=rParams.mvIn[i].mBuffer->getPlaneCount();    //total include plane count
                tmpC++;
            }
            //mvout
            tmpC=0;
            for(int i=outStartIdx;i<rParams.mvOut.size();i++)
                {
                //check cur element is next buffer group or not
                    if(tmpC==0)
                    {    //first element in this buffer group
                        curgroup=rParams.mvOut[i].mPortID.group;
                        outNum[q]=0;
                    }
                if(rParams.mvOut[i].mPortID.group != curgroup)
                {
                    outStartIdx=i;//set startIdx for next run
                    break;
                }
                //port num
                outNum[q]++;
                if(rParams.mvOut[i].mPortID.index==NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMG3O)
                    {   //img3o support multi-planes
                        outPorts+=rParams.mvOut[i].mBuffer->getPlaneCount();        //total include plane count
                    }
                    else
                    {   //others are 1 plane
                        //we do not take care multi-plane output for mdp ouput ports
                        outPorts+=1;
                    }
                    tmpC++;
            }
        }

        //statistic data, do not support slow motion + eis
        MINT32 moduletag=0;
        MINT32 inFeatureNum=0;    //input statistic dma for feature
        MINT32 outFeatureNum=0;    //output statistic dma for feature
        for(int i=0;i<rParams.mvModuleData.size();i++)
        {
            moduletag=rParams.mvModuleData[i].moduleTag;
            switch(moduletag)
            {
                case NSImageio_FrmB::NSIspio_FrmB::EP2Module_STA_FEO:
                    outPorts+=1;
                    outFeatureNum+=1;
                    break;
                case NSImageio_FrmB::NSIspio_FrmB::EP2Module_STA_LCEI:
                    inPorts+=1;
                    inFeatureNum+=1;
                    break;
                default:
                    break;
            }
        }

        if(bufPackageNum> 1 && (outFeatureNum > 1 || inFeatureNum > 1))
        {
            LOG_ERR(" DO not support slow motion + eis | vhdr");
        }
        //
        vector<NSImageio_FrmB::NSIspio_FrmB::PortInfo const*> vPostProcInPorts;
        vector<NSImageio_FrmB::NSIspio_FrmB::PortInfo const*> vPostProcOutPorts;
        NSImageio_FrmB::NSIspio_FrmB::PortInfo ports[inPorts+outPorts];
        LOG_INF("batch(%d),in/out(%d/%d)",bufPackageNum,inPorts,outPorts);

        //[5]sequencely parse buffer data
        int cnt=0;
        inStartIdx=outStartIdx=0;
        MINT32 inEndIdx=0,outEndIdx=0;
        MUINT32 dmaEnPort=0x0;
        int crop1StartIdx=0;
        MBOOL getcrop1=MFALSE;
        PipePackageInfo pipePackageInfo;
        std::vector<NSImageio_FrmB::NSIspio_FrmB::ModuleParaInfo> mvModule;
        std::vector<isp_reg_t> mvTuningReg;
        MVOID* pTuningQue=NULL;
        NSImageio_FrmB::NSIspio_FrmB::PortID rPortID(NSImageio_FrmB::NSIspio_FrmB::EPortType_Memory,NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMGI);
        NSImageio_FrmB::NSIspio_FrmB::QBufInfo    rQBufInfo;

        for(int q=0;q<bufPackageNum;q++)
        {
            inEndIdx+=inNum[q];
            outEndIdx+=outNum[q];
            vPostProcInPorts.resize(inNum[q]+inFeatureNum);        //in/out dma for each buffer node
                vPostProcOutPorts.resize(outNum[q]+outFeatureNum);
            inCnt=0;
            outCnt=0;
            dmaEnPort=0x0;
            int crop1StartIdx=0;
            getcrop1=MFALSE;
            //mvin
            for(int i=inStartIdx;i< inEndIdx ;i++)
                {
                    ports[cnt].u4Stride[0]=ports[cnt].u4Stride[1]=ports[cnt].u4Stride[2]=0;
                    planeNum = rParams.mvIn[i].mBuffer->getPlaneCount();
                    p2PortIdx=(mPortIdxMappingAlltoP2_FrmB[rParams.mvIn[i].mPortID.index].eP2PortIdx);
                    if(p2PortIdx == EPostProcPortIdx_NOSUP)
                    {
                        LOG_ERR("not support this port: %d",rParams.mvIn[i].mPortID.index); //using maping string for debug?
                        return -1;
                    }
                    else
                    {
                        //if user pass-in vipi~vip3i, we would record vipi only
                        dmaEnPort |= (1 << p2PortIdx);
                    }
                    //actually, only imgi need to set crop information
                    ports[cnt].u4ImgWidth  = rParams.mvIn[i].mBuffer->getImgSize().w;
                    ports[cnt].u4ImgHeight = rParams.mvIn[i].mBuffer->getImgSize().h;


                switch(p2PortIdx)
                    {
                        case EPostProcPortIdx_IMGI:
                            if(rParams.mvCropRsInfo.size()>0)
                            {
                                ports[cnt].crop1.x      = 0;
                                    ports[cnt].crop1.floatX = 0;
                                    ports[cnt].crop1.y      = 0;
                                    ports[cnt].crop1.floatY = 0;
                                    ports[cnt].crop1.w      = 0;
                                    ports[cnt].crop1.h      = 0;
                                    ports[cnt].resize1.tar_w= 0;
                                    ports[cnt].resize1.tar_h = 0;
                                    for(int k=crop1StartIdx;k<rParams.mvCropRsInfo.size();k++)
                                    {
                                        switch(rParams.mvCropRsInfo[k].mGroupID)
                                        {
                                                case 1:
                                                    if(getcrop1)
                                                    {
                                                        //we get crop1 of next buffer node, should jump out
                                                        crop1StartIdx=k;
                                                        k=rParams.mvCropRsInfo.size();
                                                        break;
                                                    }
                                        ports[cnt].crop1.x      = rParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                        ports[cnt].crop1.floatX = rParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                        ports[cnt].crop1.y      = rParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                        ports[cnt].crop1.floatY = rParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                        ports[cnt].crop1.w      = rParams.mvCropRsInfo[k].mCropRect.s.w;
                                        ports[cnt].crop1.h      = rParams.mvCropRsInfo[k].mCropRect.s.h;
                                        ports[cnt].resize1.tar_w = (rParams.mvCropRsInfo[k].mResizeDst.w == 0)?(rParams.mvCropRsInfo[k].mCropRect.s.w):(rParams.mvCropRsInfo[k].mResizeDst.w);
                                        ports[cnt].resize1.tar_h = (rParams.mvCropRsInfo[k].mResizeDst.h == 0)?(rParams.mvCropRsInfo[k].mCropRect.s.h):(rParams.mvCropRsInfo[k].mResizeDst.h);
                                        ports[cnt].crop2.x      = 0;
                                        ports[cnt].crop2.floatX = 0;
                                        ports[cnt].crop2.y      = 0;
                                        ports[cnt].crop2.floatY = 0;
                                        ports[cnt].crop2.w      = ports[cnt].crop1.w;
                                        ports[cnt].crop2.h      = ports[cnt].crop1.h;
                                        getcrop1=MTRUE;
                                        break;
                                                        case 2:
                                                                ports[cnt].crop2.x = rParams.mvCropRsInfo[k].mCropRect.p_integral.x;
                                                                ports[cnt].crop2.floatX = rParams.mvCropRsInfo[k].mCropRect.p_fractional.x;
                                                                ports[cnt].crop2.y = rParams.mvCropRsInfo[k].mCropRect.p_integral.y;
                                                                ports[cnt].crop2.floatY = rParams.mvCropRsInfo[k].mCropRect.p_fractional.y;
                                                                ports[cnt].crop2.w      = rParams.mvCropRsInfo[k].mCropRect.s.w;
                                                                ports[cnt].crop2.h      = rParams.mvCropRsInfo[k].mCropRect.s.h;
                                                                ports[cnt].resize2.tar_w = (rParams.mvCropRsInfo[k].mResizeDst.w == 0)?(rParams.mvCropRsInfo[k].mCropRect.s.w):(rParams.mvCropRsInfo[k].mResizeDst.w);
                                                                ports[cnt].resize2.tar_h = (rParams.mvCropRsInfo[k].mResizeDst.h == 0)?(rParams.mvCropRsInfo[k].mCropRect.s.h):(rParams.mvCropRsInfo[k].mResizeDst.h);
                                                            break;
                                            default:
                                                LOG_ERR("DO not support crop group (%d)",rParams.mvCropRsInfo[k].mGroupID);
                                                break;
                                        }
                                    }
                                }
                                else
                                {
                                    ports[cnt].crop1.x      = rParams.mvIn[i].mCropRect.p_integral.x;
                                    ports[cnt].crop1.floatX = rParams.mvIn[i].mCropRect.p_fractional.x;
                                    ports[cnt].crop1.y      = rParams.mvIn[i].mCropRect.p_integral.y;
                                    ports[cnt].crop1.floatY = rParams.mvIn[i].mCropRect.p_fractional.y;
                                    ports[cnt].crop1.w      = rParams.mvIn[i].mCropRect.s.w;
                                    ports[cnt].crop1.h      = rParams.mvIn[i].mCropRect.s.h;
                                    ports[cnt].resize1.tar_w = ports[cnt].u4ImgWidth;
                                    ports[cnt].resize1.tar_h = ports[cnt].u4ImgHeight;
                                    ports[cnt].crop2.x      = 0;
                                    ports[cnt].crop2.floatX = 0;
                                    ports[cnt].crop2.y      = 0;
                                    ports[cnt].crop2.floatY = 0;
                                    ports[cnt].crop2.w      = ports[cnt].u4ImgWidth;
                                    ports[cnt].crop2.h      = ports[cnt].u4ImgHeight;
                                    ports[cnt].resize2.tar_w = ports[cnt].u4ImgWidth;
                                    ports[cnt].resize2.tar_h = ports[cnt].u4ImgHeight;
                                }
                                LOG_DBG("[Crop] g1 x/fx/y/fy/w/h/(%d/%d/%d/%d/%d/%d)",\
                                    ports[cnt].crop1.x,ports[cnt].crop1.floatX,ports[cnt].crop1.y,ports[cnt].crop1.floatY,ports[cnt].crop1.w,ports[cnt].crop1.h);
                            break;
                        default:
                            break;
                    }
                    //
                    ports[cnt].eImgFmt     = (EImageFormat)(rParams.mvIn[i].mBuffer->getImgFormat());
                    ports[cnt].type        = mPortTypeMapping_FrmB[rParams.mvIn[i].mPortID.type].eImgIOPortType;
                    ports[cnt].index       = rParams.mvIn[i].mPortID.index; //need map to index defined in imageio
                    ports[cnt].inout       = rParams.mvIn[i].mPortID.inout;
                    ports[cnt].eImgRot     = mTransformMapping_FrmB[rParams.mvIn[i].mTransform].eImgRot;
                    ports[cnt].eImgFlip    = mTransformMapping_FrmB[rParams.mvIn[i].mTransform].eImgFlip;
                    ports[cnt].pipePass    = mSwHwPathMapping_FrmB[swScenario].ePipePass;
                    for(int k=0;k<planeNum;k++)
                    {
                    ports[cnt].u4Stride[k] = rParams.mvIn[i].mBuffer->getBufStridesInBytes(k);
                    ports[cnt].u4BufSize[k]= rParams.mvIn[i].mBuffer->getBufSizeInBytes(k);
                    ports[cnt].u4BufVA[k]  = rParams.mvIn[i].mBuffer->getBufVA(k);
                    ports[cnt].u4BufPA[k]  = rParams.mvIn[i].mBuffer->getBufPA(k);
                    ports[cnt].memID[k]    = rParams.mvIn[i].mBuffer->getFD(k);
                    }
                    //Note, Only support offset of vipi/img3o for nr3d (tpipe calculation, and nr3d + eis needed)
                    //getExtOffsetInBytes is in byte, xoffset/yoffset are in pixel for tpipe
                    switch(ports[cnt].eImgFmt)
                    {
                        case eImgFmt_YUY2:
                            pixelInByte=2;
                            break;
                        case eImgFmt_YV12:
                        default:
                            pixelInByte=1;
                            break;
                    }
                    ports[cnt].xoffset = (rParams.mvIn[i].mBuffer->getExtOffsetInBytes(0) % ports[cnt].u4Stride[0])/pixelInByte;//dma x-offset for tile calculation
                    ports[cnt].yoffset = rParams.mvIn[i].mBuffer->getExtOffsetInBytes(0) / ports[cnt].u4Stride[0];//dma y-offset for tile calculation
                    //ports[cnt].u4MagicNum  = rParams.mvMagicNo[q];
                    LOG_DBG("[mvIn (%d)] fmt(%d),w/h(%d/%d),oft(%d/%d/%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                        ports[cnt].index,ports[cnt].eImgFmt,ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,rParams.mvIn[i].mBuffer->getExtOffsetInBytes(0),\
                        ports[cnt].xoffset,ports[cnt].yoffset,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                        ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                        ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);
                    vPostProcInPorts.at(inCnt)=&(ports[cnt]);
                    cnt++;
                    inCnt++;
                }
            //mvout
                for(int i=outStartIdx;i<outEndIdx;i++)
                {
                    ports[cnt].u4Stride[0]=ports[cnt].u4Stride[1]=ports[cnt].u4Stride[2]=0;
                    planeNum = rParams.mvOut[i].mBuffer->getPlaneCount();
                    //
                    p2PortIdx=(mPortIdxMappingAlltoP2_FrmB[rParams.mvOut[i].mPortID.index].eP2PortIdx);
                    if(p2PortIdx == EPostProcPortIdx_NOSUP)
                    {
                        LOG_ERR("not support this port: %d",rParams.mvOut[i].mPortID.index);
                        return -1;
                    }
                    else
                    {
                        //if user pass-in img3o~img3co, we would record img3o only
                        dmaEnPort |= (1 << p2PortIdx);
                    }
                //
                    ports[cnt].eImgFmt     = (EImageFormat)(rParams.mvOut[i].mBuffer->getImgFormat());
                    ports[cnt].type        = mPortTypeMapping_FrmB[rParams.mvOut[i].mPortID.type].eImgIOPortType;//NSImageio_FrmB::NSIspio_FrmB::EPortType_Memory;
                    ports[cnt].index       = rParams.mvOut[i].mPortID.index; //need map to index defined in imageio
                    ports[cnt].inout       = rParams.mvOut[i].mPortID.inout;
                    ports[cnt].capbility   = rParams.mvOut[i].mPortID.capbility;
                    ports[cnt].eImgRot     = mTransformMapping_FrmB[rParams.mvOut[i].mTransform].eImgRot;
                    ports[cnt].eImgFlip    = mTransformMapping_FrmB[rParams.mvOut[i].mTransform].eImgFlip;
                    ports[cnt].pipePass    = mSwHwPathMapping_FrmB[swScenario].ePipePass;
                    ports[cnt].u4ImgWidth  = rParams.mvOut[i].mBuffer->getImgSize().w;
                    ports[cnt].u4ImgHeight = rParams.mvOut[i].mBuffer->getImgSize().h;
                    for(int k=0;k<planeNum;k++)
                    {
                    ports[cnt].u4Stride[k] = rParams.mvOut[i].mBuffer->getBufStridesInBytes(k);
                    ports[cnt].u4BufSize[k]= rParams.mvOut[i].mBuffer->getBufSizeInBytes(k);
                    ports[cnt].u4BufVA[k]  = rParams.mvOut[i].mBuffer->getBufVA(k);
                    ports[cnt].u4BufPA[k]  = rParams.mvOut[i].mBuffer->getBufPA(k);
                    ports[cnt].memID[k]    = rParams.mvOut[i].mBuffer->getFD(k);
                    }
                    //Note, Only support offset of vipi/img3o for nr3d (tpipe calculation, and nr3d + eis needed)
                    //getExtOffsetInBytes is in byte, xoffset/yoffset are in pixel for tpipe
                    switch(ports[cnt].eImgFmt)
                    {
                        case eImgFmt_YUY2:
                            pixelInByte=2;
                            break;
                        case eImgFmt_YV12:
                        default:
                            pixelInByte=1;
                            break;
                    }
                    ports[cnt].xoffset = (rParams.mvOut[i].mBuffer->getExtOffsetInBytes(0) % ports[cnt].u4Stride[0])/pixelInByte;//dma x-offset for tile calculation
                    ports[cnt].yoffset = rParams.mvOut[i].mBuffer->getExtOffsetInBytes(0) / ports[cnt].u4Stride[0];//dma y-offset for tile calculation
                //
                //ports[cnt].u4MagicNum  = rParams.mvMagicNo[q];
                LOG_DBG("[mvOut (%d)] fmt(%d),w/h(%d/%d),oft(%d/%d/%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                    ports[cnt].index,ports[cnt].eImgFmt,ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,rParams.mvOut[i].mBuffer->getExtOffsetInBytes(0),\
                    ports[cnt].xoffset,ports[cnt].yoffset,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                    ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                    ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);

                vPostProcOutPorts.at(outCnt)=&(ports[cnt]);
                cnt++;
                outCnt++;
            }

            ////////////////////////////////////////////////////////////////
            //all the additional feature data
            MBOOL isModuleSetting=MFALSE;
            MUINT32 featureEntag=0x0;
            for(int i=0;i<rParams.mvModuleData.size();i++)
            {
                moduletag=rParams.mvModuleData[i].moduleTag;
                featureEntag |= moduletag;
                switch(moduletag)
                {
                    case NSImageio_FrmB::NSIspio_FrmB::EP2Module_STA_FEO:
                    case NSImageio_FrmB::NSIspio_FrmB::EP2Module_STA_LCEI:
                        isModuleSetting=MFALSE;
                        break;
                    case NSImageio_FrmB::NSIspio_FrmB::EP2Module_NR3D:
                    case NSImageio_FrmB::NSIspio_FrmB::EP2Module_SRZ1:
                        isModuleSetting=MTRUE;
                        break;
                    default:
                        break;
                }
                //
                if(isModuleSetting)
                {
                    LOG_DBG("[mvModule] moduleEnum(%d)",moduletag);
                    NSImageio_FrmB::NSIspio_FrmB::ModuleParaInfo module;
                    module.eP2module=(NSImageio_FrmB::NSIspio_FrmB::EP2Module)(rParams.mvModuleData[i].moduleTag);
                    module.moduleStruct=rParams.mvModuleData[i].moduleStruct;
                    mvModule.push_back(module);
                }
                else
                {
                    ports[cnt].u4Stride[0]=ports[cnt].u4Stride[1]=ports[cnt].u4Stride[2]=0;
                        p2PortIdx=(mPortIdxMappingAlltoP2_FrmB[reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->port_idx].eP2PortIdx);
                        if(p2PortIdx == EPostProcPortIdx_NOSUP)
                        {
                            LOG_ERR("not support this port: %d",reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->port_idx); //using maping string for debug?
                            return -1;
                        }
                        else
                        {
                            bufPackage.portEn |= (1 << p2PortIdx);
                        }
                        ports[cnt].eImgFmt     = eImgFmt_BAYER8;
                        ports[cnt].u4ImgWidth  = reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->w;//rFeatureData.feoData.w;
                        ports[cnt].u4ImgHeight = reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->h;//rFeatureData.feoData.h;
                        ports[cnt].u4Stride[0] = reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->stride;//rFeatureData.feoData.stride;//stride in bytes
                        ports[cnt].u4Stride[1] = 0;
                        ports[cnt].u4Stride[2] = 0;
                        ports[cnt].type        = reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->port_type;
                        ports[cnt].index       = reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->port_idx;
                        ports[cnt].inout       = reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->port_inout;
                        ports[cnt].eImgRot     = NSImageio_FrmB::NSIspio_FrmB::eImgRot_0; //no support rotate
                        ports[cnt].eImgFlip    = NSImageio_FrmB::NSIspio_FrmB::eImgFlip_OFF; //no support fliip
                        ports[cnt].pipePass    = mSwHwPathMapping_FrmB[swScenario].ePipePass;
                        ports[cnt].u4BufVA[0]  = reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->bufInfo.virtAddr;
                        ports[cnt].u4BufPA[0]  = reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->bufInfo.phyAddr;
                        ports[cnt].memID[0]    = reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->bufInfo.memID;
                        ports[cnt].u4BufSize[0]= reinterpret_cast<StaDataCfg*>(rParams.mvModuleData[i].moduleStruct)->bufInfo.size;
                        ports[cnt].u4MagicNum  = rParams.mFrameNo;

                    //
                        switch(moduletag)
                    {
                        case NSImageio_FrmB::NSIspio_FrmB::EP2Module_STA_FEO:
                            LOG_DBG("[mvOut (%d)] fmt(%d),w(%d),h(%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                                ports[cnt].index,ports[cnt].eImgFmt,ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                                    ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                                      ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);
                                vPostProcOutPorts.at(outCnt)=&(ports[cnt]);
                                cnt++;
                                outCnt++;
                                            break;
                        case NSImageio_FrmB::NSIspio_FrmB::EP2Module_STA_LCEI:
                            LOG_DBG("[mvIn (%d)] fmt(%d),w(%d),h(%d),stride(%d/%d/%d),cap(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )",\
                                       ports[cnt].index,ports[cnt].eImgFmt,ports[cnt].u4ImgWidth,ports[cnt].u4ImgHeight,ports[cnt].u4Stride[0],ports[cnt].u4Stride[1],ports[cnt].u4Stride[2],\
                                        ports[cnt].capbility,ports[cnt].u4BufVA[0],ports[cnt].u4BufVA[1],ports[cnt].u4BufVA[2],ports[cnt].u4BufPA[0],ports[cnt].u4BufPA[1],ports[cnt].u4BufPA[2],\
                                        ports[cnt].u4BufSize[0],ports[cnt].u4BufSize[1],ports[cnt].u4BufSize[2]);
                                vPostProcInPorts.at(inCnt)=&(ports[cnt]);
                                cnt++;
                                inCnt++;
                            break;
                        default:
                            break;
                    }
                }
            }


                //[5]config pipe
                if(rParams.mvPrivaData[q])
                    {
                    LOG_INF("dmaEnPort(0x%08x),featureEntag (0x%x),q(%d),pvd(0x%x)",dmaEnPort,featureEntag,q,rParams.mvPrivaData[q]);
                    }
                   else
                    {
                    LOG_INF("dmaEnPort(0x%08x),featureEntag (0x%x),q(%d)",dmaEnPort,featureEntag,q);
                    }


            // process p2 tuning setting only for camera3.0
            if(misV3){
                if(rParams.mpTuningData.size()==0) {
                    LOG_ERR("[Error]PLZ add tuning data inv3");
                    return MFALSE;
                } else {
                    pTuningQue = rParams.mpTuningData[q];
                    LOG_DBG("q(%d),pTuningQue(0x%x)",q,pTuningQue);
                }
                //
                if(rParams.mvP1SrcCrop.size()==0 || rParams.mvP1Dst.size()==0 || rParams.mvP1DstCrop.size()==0) {
                    LOG_ERR("[Error]PLZ add mvP1SrcCrop/mvP1Dst/mvP1DstCrop inv3");
                    return MFALSE;
                } else {
                    pipePackageInfo.mP1PrivateData.ResizeSrcW = rParams.mvP1SrcCrop[q].s.w;
                    pipePackageInfo.mP1PrivateData.ResizeSrcH = rParams.mvP1SrcCrop[q].s.h;
                    pipePackageInfo.mP1PrivateData.ResizeDstW = rParams.mvP1Dst[q].w;
                    pipePackageInfo.mP1PrivateData.ResizeDstH = rParams.mvP1Dst[q].h;
                    pipePackageInfo.mP1PrivateData.ResizeCropX = rParams.mvP1DstCrop[q].p.x;
                    pipePackageInfo.mP1PrivateData.ResizeCropY = rParams.mvP1DstCrop[q].p.y;
                    //
                    LOG_DBG("mP1PrivateData, crop(%d,%d),src(%d,%d),dst(%d,%d)", \
                        pipePackageInfo.mP1PrivateData.ResizeCropX, pipePackageInfo.mP1PrivateData.ResizeCropY, \
                        pipePackageInfo.mP1PrivateData.ResizeSrcW,pipePackageInfo.mP1PrivateData.ResizeSrcH, \
                        pipePackageInfo.mP1PrivateData.ResizeDstW,pipePackageInfo.mP1PrivateData.ResizeDstH);
                }
            }
            //
            // fill in pPipePackageInfo with package data
            pipePackageInfo.burstQIdx = q;
            pipePackageInfo.dupCqIdx = bufPackage.p2cqDupIdx;
            pipePackageInfo.pCookie = rParams.mvPrivaData[q];
            pipePackageInfo.vModuleParams = mvModule;
            pipePackageInfo.pTuningQue = pTuningQue;
            pipePackageInfo.isV3 = misV3;
            pipePackageInfo.pixIdP2 = mpixIdP2;

            //*******************************************************************//
            ret=callerPostProc->configPipe(vPostProcInPorts, vPostProcOutPorts, &pipePackageInfo);
            if(!ret)
            {
                LOG_ERR("postprocPipe config fail");
                return ret;
            }
                //*******************************************************************//

            //[6] trigger mdp start
                ret=callerPostProc->start();
            if(!ret)
            {
                LOG_ERR("P2 Start Fail!");
                return ret;
            }

            //[7] add enqueued buf info to kernel after cofig pipe, to make sure the buffer order in kernel list is the same with we send to mdp
            bufPackage.callerID=(MUINTPTR)(&callerDeQList);
            bufPackage.bufTag=bufferTag;
            LOG_DBG("ecID(0x%x),tag(%d)",bufPackage.callerID,bufPackage.bufTag);


            if(inStartIdx==0)
            {
                //top-level record number of buffer-batches
                bufPackage.rParams=(rParams);
                bufPackage.drvSce=mSwHwPathMapping_FrmB[swScenario].hwPath;
                /******************************* BufBatch Lock Region Start ********************************/
                getLock(ELockEnum_BufBatchEnQ);
                //top-level buffer-batches list
                bufPackage.idxINbatchLforBnode=mvEnqueuedBufBatch.size();    //get real current index in batch list (HalpipeWrapper.Thread would delete element from batch list if deque success)
                mvEnqueuedBufBatch.push_back(bufPackage);

                //second-level record number of buffer nodes(one buffer batch may include lots of buffer nodes)
                getLock(ELockEnum_BufNodeEnQ);
                BufParamNode bufNode;
                bufNode.portEn=dmaEnPort;
                bufNode.p2cqIdx=bufPackage.p2cqIdx;
                bufNode.p2cqDupIdx=bufPackage.p2cqDupIdx;
                bufNode.p2BurstQIdx=q;
                bufNode.bufTag=bufPackage.bufTag;
                bufNode.drvSce=bufPackage.drvSce;
                bufNode.callerID=bufPackage.callerID;
                bufNode.idxInBufBatchList=bufPackage.idxINbatchLforBnode;        //avoid timing issue, addBatch -> addNode -> removeBatch
                bufNode.jpgo_idx=bufPackage.jpgo_idx;
                mvEnqueueBufNode.push_back(bufNode);
                //for user, they only know callback or deque done
                ret=callerPostProc->endequeFrameCtrl(NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_ENQUE_FRAME,bufNode.callerID,bufNode.p2BurstQIdx,bufPackage.p2cqDupIdx);
                releaseLock(ELockEnum_BufNodeEnQ);
                releaseLock(ELockEnum_BufBatchEnQ);
                /******************************* BufBatch Lock Region End ********************************/

                getLock(ELockEnum_CQUser);
                mp2CQDupIdx[pathCQ]=1-mp2CQDupIdx[pathCQ];  //pinpon for next user
                mp2CQUserNum[pathCQ]++;  //update current user number
                releaseLock(ELockEnum_CQUser);
            }
            else
            {
                BufParamNode bufNode;
                getLock(ELockEnum_BufBatchEnQ);
                //get real current index in batch list (HalpipeWrapper.Thread would delete element from batch list if deque success)
                //FIXME, if there is the scenario that buffer nodes of two different buffer batches are cross enque ?
                //eleMentidx=mvEnqueuedBufBatch.size()-1;

                //second-level record number of buffer nodes(one buffer batch may include lots of buffer nodes)
                getLock(ELockEnum_BufNodeEnQ);

                bufNode.portEn=dmaEnPort;
                bufNode.p2cqIdx=bufPackage.p2cqIdx;
                bufNode.p2cqDupIdx=bufPackage.p2cqDupIdx;
                bufNode.p2BurstQIdx=q;
                bufNode.bufTag=bufPackage.bufTag;
                bufNode.drvSce=bufPackage.drvSce;
                bufNode.callerID=bufPackage.callerID;
                if(bufPackage.idxINbatchLforBnode == mvEnqueuedBufBatch.size())    //update real index for waited added buffer node in batchA
                { bufNode.idxInBufBatchList = mvEnqueuedBufBatch[mvEnqueuedBufBatch.size()-1].idxINbatchLforBnode; }
                else
                { bufNode.idxInBufBatchList = bufPackage.idxINbatchLforBnode; }

                bufNode.jpgo_idx=bufPackage.jpgo_idx;
                mvEnqueueBufNode.push_back(bufNode);
                //for user, they only know callback or deque done
                ret=callerPostProc->endequeFrameCtrl(NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_ENQUE_FRAME,bufNode.callerID,bufNode.p2BurstQIdx,bufPackage.p2cqDupIdx);
                releaseLock(ELockEnum_BufNodeEnQ);
                releaseLock(ELockEnum_BufBatchEnQ);
                if(!ret)
                {
                    LOG_ERR("enque ioctrl fail, return");
                    return ret;
                }
            }

            //[8] send signal to dequeue thread that another user enqueue a buf
            #if 1
            ECmd enqeueCmd=ECmd_ENQUEUE;
            sendCommand(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_P2_QUEUE_CONTROL_STATE, (MUINT32)pathCQ, (MUINT32)enqeueCmd);
            #else
            ECmd enqeueCmd=ECmd_ENQUEUE;
            sendCommand((int)pathCQ,enqeueCmd);
            #endif

            //[9] pop out local variables forr next turn
            for(int i=(int)vPostProcInPorts.size()-1;i>=0;i--)
                vPostProcInPorts.pop_back();
            for(int i=(int)vPostProcOutPorts.size()-1;i>=0;i--)
                vPostProcOutPorts.pop_back();
            for(int i=(int)mvModule.size()-1;i>=0;i--)
                mvModule.pop_back();

            //[10]update starting parse index
            inStartIdx+=inNum[q];
            outStartIdx+=outNum[q];
        }
    }

    LOG_DBG("-");
    return ret;

}
/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HalPipeWrapper::
deque(
    QParams& rParams,
    NSImageio_FrmB::NSIspio_FrmB::IPostProcPipe *callerPostProc,
    MUINT32 callerID,
    MINT64 i8TimeoutNs)
{
    bool ret = MFALSE;

    MINT32 timeout=i8TimeoutNs/1000;
    LOG_DBG("timeout(%d),i8TimeoutNs(%lld),callerID(0x%x),de_frameNo(%d)",timeout,i8TimeoutNs,callerID,rParams.mFrameNo);

    //[1] goto check if buffer ready
    MUINT32 a,b;
    a=getUs();
    //assume that first enqued p2 dupCQ first out, always pass 0 as p2dupCQidx when deque
    ret=callerPostProc->endequeFrameCtrl(NSImageio_FrmB::NSIspio_FrmB::EPIPE_BUFQUECmd_WAIT_FRAME,callerID,0, 0 , timeout);
    b=getUs();
    LOG_INF("===== ret/period(%d/%d us) =====",ret,b-a);
    //[2] get the first matched element in dequeue buffer list
    if(ret)
    {
        queryFrame(rParams,callerID);
        return MTRUE;
    }
    else
    {
        return MFALSE;
    }
}
/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
queryFrame(
    QParams& params,
    MINT32 callerID)
{
    bool ret=false;
    FUNCTION_LOG_START;
    int cnt=0;
    getLock(ELockEnum_BufBatchDeQ);
    LOG_INF("cID(%d) dBufSize(%d)",params.mFrameNo,(int)(mvDequeuedBufBatch.size()));
    for (list<BufParamPackage>::iterator it = mvDequeuedBufBatch.begin(); it != mvDequeuedBufBatch.end();)
    {
        if((*it).callerID == callerID)
        {   //get matched buffer
            params=(*it).rParams;
            mvDequeuedBufBatch.erase(it);
            LOG_INF("dequeuedBufListSize(%d)",(int)(mvDequeuedBufBatch.size()));
            ret=true;
            break;
        }
        it++;
        cnt++;
    }
    releaseLock(ELockEnum_BufBatchDeQ);
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
startVideoRecord(MINT32 wd,MINT32 ht,MINT32 fps, ESoftwareScenario swScen)
{
    bool ret=false;
    FUNCTION_LOG_START;
    #if 0
    getLock(ELockEnum_VencCnt);
    mVencPortCnt=0;
    LOG_DBG("mVencPortCnt(%d)",mVencPortCnt);
    releaseLock(ELockEnum_VencCnt);
    if(mpPostProcPipe)
    {
        NSImageio_FrmB::NSIspio_FrmB::EDrvScenario eDrv=mSwHwPathMapping_FrmB[swScen].hwPath;
        MUINT32 cqIdx=0x0;
        switch(mSwHwPathMapping_FrmB[swScen].ePathCQ)
        {
            case EPathCQ_CQ1:
                cqIdx=NSIspDrv_FrmB::ISP_DRV_CQ01;
                break;
            case EPathCQ_CQ2:
                cqIdx=EDrvCQ_CQ02;
                break;
            case EPathCQ_CQ3:
                cqIdx=EDrvCQ_CQ03;
                break;
            default:
                LOG_ERR("no support this p2 path(%d)",mSwHwPathMapping_FrmB[swScen].ePathCQ);
                break;
        }
        ret=mpPostProcPipe->startVideoRecord(wd,ht,fps,eDrv,cqIdx);
        if(!ret)
        {
            LOG_ERR("startVideo Record Fail");
        }
    }
    else
    {
        LOG_ERR("plz do init first");
    }
    FUNCTION_LOG_END;
    #else
    LOG_ERR("do not support this function now ");
    #endif
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
stopVideoRecord(ESoftwareScenario swScen)
{
    bool ret=false;
    #if 0
    FUNCTION_LOG_START;
    //[1] wait all the buffer-batches containing venc port are deque done
    getLock(ELockEnum_VencCnt);
    LOG_INF("+ mVencPortCnt(%d)",mVencPortCnt);
    if(mVencPortCnt>0)
    {
        releaseLock(ELockEnum_VencCnt);
        while(1)
        {
            ::sem_wait(&mSemVencCnt);
            getLock(ELockEnum_VencCnt);
            LOG_DBG("* mVencPortCnt(%d)",mVencPortCnt);
            if(mVencPortCnt==0)
            {
                releaseLock(ELockEnum_VencCnt);
                break;
            }
            else
            {
                releaseLock(ELockEnum_VencCnt);
            }
        }

    }
    else
    {
        releaseLock(ELockEnum_VencCnt);
    }

    //[2] do stop video record
    LOG_INF("- mVencPortCnt(%d)",mVencPortCnt);
    if(mpPostProcPipe)
    {
        NSImageio_FrmB::NSIspio_FrmB::EDrvScenario eDrv=mSwHwPathMapping_FrmB[swScen].hwPath;
        MUINT32 cqIdx=0x0;
        switch(mSwHwPathMapping_FrmB[swScen].ePathCQ)
        {
            case EPathCQ_CQ1:
                cqIdx=NSIspDrv_FrmB::ISP_DRV_CQ01;
                break;
            case EPathCQ_CQ2:
                cqIdx=EDrvCQ_CQ02;
                break;
            case EPathCQ_CQ3:
                cqIdx=EDrvCQ_CQ03;
                break;
            default:
                LOG_ERR("no support this p2 path(%d)",mSwHwPathMapping_FrmB[swScen].ePathCQ);
                break;
        }
        ret=mpPostProcPipe->stopVideoRecord(eDrv,cqIdx);
        if(!ret)
        {
            LOG_ERR("stopVideo Record Fail");
        }
    }
    else
    {
        LOG_ERR("plz do init first");
    }
    FUNCTION_LOG_END;
    #else
    LOG_ERR("do not support this function now ");
    #endif
    return ret;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
sendCommand(
    MINT32 cmd,
    MINTPTR arg1,
    MINTPTR arg2,
    MINTPTR arg3)
{

    LOG_DBG("+,cmd(0x%x),arg1(0x%x),arg2(0x%x),arg3(0x%x)",cmd,arg1,arg2,arg3);
    switch (cmd){
        case NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_P2_QUEUE_CONTROL_STATE:
            if((ECmd)arg2 == ECmd_ENQUEUE){
                // signal to vss buffer if there is any incoming buffer (actually user should not enque any vss buffer if there still exist vss buffer in list)
                {   //the life cycle of mVssCondMtx is in the following region
                    Mutex::Autolock autoLock(mVssCondMtx);
                    mVssCond.signal();
                }
                // add enque cmd
                addCommand(arg1,ECmd_ENQUEUE);  // arg1 = dequeCq
            }
            break;
        case NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_GET_NR3D_GAIN:
            {
                Mutex::Autolock lock(mEnqueLock);
                mpPostProcPipe->sendCommand(cmd, arg1, arg2, arg3);
            }
            break;
        default:
            mpPostProcPipe->sendCommand(cmd, arg1, arg2, arg3);
            break;

    }
    return true;
}

/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
deTuningQue(
    unsigned int& size,
    void* &pTuningQueBuf)
{
    Mutex::Autolock lock(mTuningQueLock);
    bool ret=true;
    //
    if(misV3)
    {
        size = sizeof(isp_reg_t);
        if(!gTuningQueNode.empty()) {
            pTuningQueBuf = gTuningQueNode[0];
            gTuningQueNode.erase(gTuningQueNode.begin());
            //
            LOG_DBG("queNum(%d),detuningQue(0x%08x)\n",gTuningQueNode.size(),pTuningQueBuf);
        } else {
            tuninQueData tuningQueDataNew;
            #if defined(__PMEM_ONLY__)
                tuningQueDataNew.bufferInfo.virtAddr = (MUINT32*)pmem_alloc_sync(size, &gTuningQueData[i].bufferInfo.memID);
                memset((MUINT8*)tuningQueDataNew.bufferInfo.virtAddr,0,size);
            #else     // Not PMEM.
                tuningQueDataNew.bufferInfo.size = size;
                tuningQueDataNew.bufferInfo.useNoncache = 0; //alloc cacheable mem.
                if ( m_pIMemDrv->allocVirtBuf(&tuningQueDataNew.bufferInfo) ) {
                    LOG_ERR("[ERROR]:m_pIMemDrv->allocVirtBuf");
                }
                memset((MUINT8*)tuningQueDataNew.bufferInfo.virtAddr,0,size);
            #endif
            gTuningQueData.push_back(tuningQueDataNew);
            gTuningQueNode.push_back((void*)tuningQueDataNew.bufferInfo.virtAddr);
            //
            pTuningQueBuf = gTuningQueNode[0];
            gTuningQueNode.erase(gTuningQueNode.begin());
            //
            mTuningQueNum++;
            LOG_DBG("[Warning]out of tuningque, add one more(0x%x)\n",gTuningQueData[gTuningQueData.size()-1].bufferInfo.virtAddr);
        }

        LOG_DBG("[size]gTuningQueData(%d),gTuningQueNode(%d)\n",gTuningQueData.size(),gTuningQueNode.size());
    }
    else
    {
        LOG_ERR("v1 do not support this function, misV3(%d),misV3");
    }
    //
    return ret;
}
/******************************************************************************
*
******************************************************************************/
MBOOL
HalPipeWrapper::
enTuningQue(
    void* pTuningQueBuf)
{
        Mutex::Autolock lock(mTuningQueLock);
    bool ret=true;

    if(misV3)
    {
        bool isCheckPass = false;
        //
        for(MUINT32 i=0;i<gTuningQueData.size();i++){
            if((void*)gTuningQueData[i].bufferInfo.virtAddr == pTuningQueBuf) {
                isCheckPass = true;
                break;
            }
        }
        // check tuningquebuufer address that get from M/W
        if(isCheckPass == true){
            gTuningQueNode.push_back(pTuningQueBuf);
        }else{
            LOG_ERR("[Error]tuningque buffer(0x%08x) is invalid",pTuningQueBuf);
        }
        //
        for(MUINT32 i=0;i<gTuningQueNode.size();i++){
            LOG_DBG("i(%d),pmem(0x%08x)\n",i,gTuningQueNode[i]);
        }
    }
    else
    {
        LOG_ERR("v1 do not support this function, misV3(%d),misV3");
    }
    //
    return ret;
}


