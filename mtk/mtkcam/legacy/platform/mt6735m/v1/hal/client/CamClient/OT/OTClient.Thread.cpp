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

#define LOG_TAG "MtkCam/CamClient/OTClient"
//
#include "OTClient.h"
using namespace NSCamClient;
using namespace NSOTClient;
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <cutils/properties.h>
//
#include <mtkcam/hwutils/CameraProfile.h>  // For CPTLog*()/AutoCPTLog class.
#include "camera_custom_ot.h"

#if '1'==MTKCAM_HAVE_3A_HAL
#include <CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
#include <mtkcam/hal/aaa_hal_base.h>
using namespace NS3A;
#endif

static int DROP_FRAME = 5;
static int g_FrameCount = 0;

/******************************************************************************
*
*******************************************************************************/
#define ENABLE_LOG_PER_FRAME        (1)

/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)[%s] " fmt, ::gettid(), __FUNCTION__, ##arg)
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
// Ask this object's thread to exit. This function is asynchronous, when the
// function returns the thread might still be running. Of course, this
// function can be called from a different thread.
void
OTClient::
requestExit()
{
    MY_LOGD("+");
    Thread::requestExit();
    //
    postCommand(Command::eID_EXIT);
    //
    MY_LOGD("-");
}


/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
status_t
OTClient::
readyToRun()
{
    ::prctl(PR_SET_NAME,"OTClient@Preview", 0, 0, 0);
    //
    mi4ThreadId = ::gettid();

    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    int const policy    = SCHED_OTHER;
    int const priority  = 0;
    //
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    setpriority(PRIO_PROCESS, 0, priority);
    //
    //
    MY_LOGD(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, getpriority(PRIO_PROCESS, 0)
    );

    return NO_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
void
OTClient::
postCommand(Command::EID cmd)
{
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    if  ( ! mCmdQue.empty() )
    {
        Command::EID const& rBegCmd = *mCmdQue.begin();
        MY_LOGW("que size:%d > 0 with begin cmd::%d", mCmdQue.size(), rBegCmd);
    }
    //
    mCmdQue.push_back(cmd);
    mCmdQueCond.broadcast();
    //
    MY_LOGD("- new command::%d", cmd);
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
getCommand(Command::EID &cmd)
{
    bool ret = false;
    //
    Mutex::Autolock _lock(mCmdQueMtx);
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "+ que size(%d)", mCmdQue.size());
    //
    //  Wait until the queue is not empty or this thread will exit.
    while   ( mCmdQue.empty() && ! exitPending() )
    {
        status_t status = mCmdQueCond.wait(mCmdQueMtx);
        if  ( NO_ERROR != status )
        {
            MY_LOGW("wait status(%d), que size(%d), exitPending(%d)", status, mCmdQue.size(), exitPending());
        }
    }
    //
    if  ( ! mCmdQue.empty() )
    {
        //  If the queue is not empty, take the first command from the queue.
        ret = true;
        cmd = *mCmdQue.begin();
        mCmdQue.erase(mCmdQue.begin());
        MY_LOGD("command:%d", cmd);
    }
    //
    MY_LOGD_IF(ENABLE_LOG_PER_FRAME, "- que size(%d), ret(%d)", mCmdQue.size(), ret);
    return  ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
OTClient::
threadLoop()
{
    Command::EID cmd;
    if  ( getCommand(cmd) )
    {
        switch  (cmd)
        {
        case Command::eID_WAKEUP:
            onClientThreadLoop();
            break;
        //
        case Command::eID_EXIT:
        default:
            MY_LOGD("Command::%d", cmd);
            break;
        }
    }
    //
    MY_LOGD("-");
    return  true;
}


/******************************************************************************
 *
 ******************************************************************************/
void
OTClient::
onClientThreadLoop()
{
    MY_LOGD("+");

    //(0) pre-check
    sp<IImgBufQueue> pBufQueue = NULL;
    {
        //Mutex::Autolock _l(mModuleMtx);
        //
        pBufQueue = mpImgBufQueue;
        if ( pBufQueue == 0 || ! isEnabledState())
        {
            MY_LOGE("pBufMgr(%p), isEnabledState(%d)", pBufQueue.get(), isEnabledState());
            return;
        }
    }

    //(1) prepare all TODO buffers
    if ( ! initBuffers(pBufQueue) )
    {
        MY_LOGE("initBuffers failed");
        return;
    }

    //(2) start
    if  ( !pBufQueue->startProcessor() )
    {
        MY_LOGE("startProcessor failed");
        return;
    }

    //BinChang 2013/10/01
    //Lock AE, AWB

     #if '1'==MTKCAM_HAVE_3A_HAL

     OT_Customize_PARA OTCustomData1;
     get_ot_CustomizeData(&OTCustomData1);
     MY_LOGD("Custom Data: AE_AWB lock=%d", OTCustomData1.AEAWB_LOCK);
     if(OTCustomData1.AEAWB_LOCK==1)
     {
        NS3A::Param_T cam3aParam;
        Hal3ABase* p3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(mpParamsMgr->getOpenId()));
        //IHal3A* p3AHal = IHal3A::createInstance(NS3A::IHal3A::E_Camera_1, mpParamsMgr->getOpenId(), LOG_TAG);

        if (p3AHal)
        {
//           p3AHal->getParams(cam3aParam);
//          cam3aParam.bIsAELock = 1;
//           cam3aParam.bIsAWBLock = 1;
//           p3AHal->setParams(cam3aParam);
           p3AHal->setAELock(1);
           p3AHal->setAWBLock(1);
           p3AHal->destroyInstance();
           p3AHal = NULL;

           MY_LOGD("AE_AWB lock");
        }
     }
     #endif

    //BinChang 2013/07/10
    //Initial OT

    int srcWidth=0,  srcHeight=0;
    int pv_srcWidth =0, pv_srcHeight = 0;
    mpParamsMgr->getPreviewSize(&pv_srcWidth, &pv_srcHeight);

    /*srcWidth = 640;
    if(pv_srcWidth != 0)
        srcHeight = srcWidth * pv_srcHeight / pv_srcWidth;
    else
        srcHeight = 480;*/
    srcWidth = pv_srcWidth;
    srcHeight = pv_srcHeight;

    mpOTHalObj = halOTBase::createInstance(HAL_OT_OBJ_SW);
    mpOTHalObj->halOTInit(srcWidth, srcHeight, 0, 0);

    //(3) Do in loop until stopObjectTracking has been called
    //    either by sendCommand() or by stopPreview()
    while ( isEnabledState() )
    {
        // (3.1) deque from processor
        ImgBufQueNode rQueNode;
        if ( ! waitAndHandleReturnBuffers(pBufQueue, rQueNode) )
        {
            MY_LOGD("No available deque-ed buffer; to be leaving");
            continue;
        }

        if ( rQueNode.getImgBuf() == 0 )
        {
            MY_LOGE("rQueNode.getImgBuf() == 0");
            continue;
        }

        if( (unsigned int *)(rQueNode.getImgBuf()->getVirAddr() != VA1) && ((unsigned int *)rQueNode.getImgBuf()->getVirAddr() != VA2) )
        {
            MY_LOGD("Deque VA Incorrect!, Deque VA:0x%x, VA1:0x%x, VA2, 0x%x", rQueNode.getImgBuf()->getVirAddr(), VA1, VA2);
            continue;
        }

        // (3.2) do OT algorithm
        bool isDetected_OT = false;

        int const i4CamMode = mpParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
        if  ( i4CamMode == MtkCameraParameters::CAMERA_MODE_NORMAL )
            Rotation_Info = 180;
        else
            Rotation_Info = mpParamsMgr->getInt(CameraParameters::KEY_ROTATION);

        //CPTLog(Event_Hal_Client_CamClient_OT, CPTFlagStart);  // Profiling Start.
        if ( ! isMsgEnabled() )
        {
            MY_LOGD("Don't do OT");
        }
        else
        {
            g_FrameCount++;
            if(g_FrameCount <= DROP_FRAME)
            {
               isDetected_OT = false;
               MY_LOGD("Initial Frame Count: %d", g_FrameCount);
            }

            else
            {
               if ( ! doOT(rQueNode, isDetected_OT) )
               {
                  MY_LOGE("doOT failed");
                  //CPTLog(Event_Hal_Client_CamClient_OT, CPTFlagEnd);  // Profiling End.
                  continue;
               }

            }
        }

        //        CPTLog(Event_Hal_Client_CamClient_OT, CPTFlagEnd);    // Profiling End.


        if(g_FrameCount <= DROP_FRAME)
        {
            MY_LOGD("No setOTInfoTo3A Frame Count: %d", g_FrameCount);
        }
        else
        {
            // (3.3)
            #if '1'==MTKCAM_HAVE_3A_HAL
            Hal3ABase* p3AHal = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(mpParamsMgr->getOpenId()));
            //IHal3A* p3AHal = IHal3A::createInstance(NS3A::IHal3A::E_Camera_1, mpParamsMgr->getOpenId(), LOG_TAG);
            if (p3AHal)
            {
                p3AHal->setOTInfo(mpDetectedObjs);
                p3AHal->destroyInstance();
            }
            #endif
        }

        // (3.4)
        if(g_FrameCount <= DROP_FRAME)
        {
            MY_LOGD("No Callback Frame Count: %d", g_FrameCount);
        }
        else
        {
            performCallback(isDetected_OT);
        }

        // (3.5)
        // enque back to processor

        //MY_LOGD("Before enque: Virtual Addr: 0x%x, ", rQueNode.getImgBuf()->getVirAddr());

        handleReturnBuffers(pBufQueue, rQueNode); //enque to "TODO"
    }

#if (0) //After pauseprocessor, deque is useless
    MY_LOGD("Deque In mbuf_count:%d", mbuf_count);
    while(mbuf_count!=mBufCnt) {
       Vector<ImgBufQueNode> vQueNode;
       //
       pBufQueue->dequeProcessor(vQueNode);
       //

       for (size_t t = 0; t < vQueNode.size(); t++)
       {
         MY_LOGD("After deque:0x%x", vQueNode[t].getImgBuf()->getVirAddr());
       }
       mbuf_count++;
    }
    MY_LOGD("Deque Out mbuf_count:%d", mbuf_count);
#endif

    g_FrameCount = 0;

    if  ( mpOTHalObj != 0 )
    {
        mpOTHalObj->halOTUninit();
        mpOTHalObj->destroyInstance();
        mpOTHalObj = NULL;
    }

    //(4) stop.
    pBufQueue->pauseProcessor();
    pBufQueue->flushProcessor(); // clear "TODO"
    pBufQueue->stopProcessor();  // clear "DONE"

#if '1'==MTKCAM_HAVE_3A_HAL
    NS3A::Param_T cam3aParam1;
    Hal3ABase* p3AHal1 = Hal3ABase::createInstance(DevMetaInfo::queryHalSensorDev(mpParamsMgr->getOpenId()));
    //IHal3A* p3AHal1 = IHal3A::createInstance(NS3A::IHal3A::E_Camera_1, mpParamsMgr->getOpenId(), LOG_TAG);
    if (p3AHal1)
    {
        mpDetectedObjs->number_of_faces = 0;
        p3AHal1->setOTInfo(mpDetectedObjs);
//        p3AHal1->getParams(cam3aParam1);
//        cam3aParam1.bIsAELock = 0;
//        cam3aParam1.bIsAWBLock = 0;
//        p3AHal1->setParams(cam3aParam1);
        p3AHal1->setAELock(0);
        p3AHal1->setAWBLock(0);
        p3AHal1->destroyInstance();
        p3AHal1 = NULL;
        MY_LOGD("AE_AWB unlock");
    }
#endif

    uninitBuffers();

    MY_LOGD("-");
}

