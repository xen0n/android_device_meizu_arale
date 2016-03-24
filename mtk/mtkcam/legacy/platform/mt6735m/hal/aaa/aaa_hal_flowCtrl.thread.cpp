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
#define LOG_TAG "Hal3AFlowCtrl/thread"

#include <utils/threads.h>
#include <utils/List.h>
#include <mtkcam/v1/config/PriorityDefs.h>
#include <mtkcam/utils/common.h>
using namespace android;
using namespace NSCam::Utils;

//
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <hal/aaa/aaa_hal_flowCtrl.h>
#include <hal/aaa/IEventIrq.h>
#include <mtkcam/Trace.h>

using namespace NS3Av3;
//
//
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

/******************************************************************************
*
*******************************************************************************/
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG   (1)
#endif
#define LOG_PERFRAME_ENABLE (1)

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif



/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3AFlowCtrl::createThread()
{
    sem_init(&mSem, 0, 0);
    //sem_init(&semAFProcEnd, 0, 0);
    pthread_create(&mThread, NULL, onThreadLoop, this);
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3AFlowCtrl::destroyThread()
{
    MY_LOG("+");

    // post exit
    addCommandQ(ECmd_Uninit);

    //
    pthread_join(mThread, NULL);

    //
    MY_LOG("-");
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3AFlowCtrl::changeThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME,"3ATHREAD", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_3A_MAIN;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //
        MY_LOG(
            "[Hal3AFlowCtrl::onThreadLoop] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , expect_policy, policy, expect_priority, priority
        );
    }

}


/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3AFlowCtrl::sendCommand(ECmd_T const r3ACmd, MINTPTR const i4Arg)
{
    MBOOL bRet = MTRUE;

    mbListenUpdate = (r3ACmd == ECmd_CameraPreviewStart) ? MTRUE :
                     (r3ACmd == ECmd_CameraPreviewEnd)   ? MFALSE : mbListenUpdate;

    sendCommandDerived(r3ACmd, i4Arg);
/*
    if (mpStateMgr->getStateStatus().eCurrState == eState_AF)
        if ((r3ACmd == ECmd_RecordingStart) || (r3ACmd == ECmd_RecordingEnd) || (r3ACmd == ECmd_CameraPreviewEnd))
        {
            MY_LOG_IF(1, "[Hal3AFlowCtrl::sendCommand] eState_AF: get r3ACmd(%d)", r3ACmd);
            cancelAutoFocus();
        }

#if 1 //move back from setIspProfile(), due to command order change
    if ((r3ACmd == ECmd_CameraPreviewStart) || (r3ACmd == ECmd_CaptureStart))
        queryTGInfoFromSensorHal(); //update TG info for VS irq configuration. This should be done after MW config sensor AND before ECmd_Update is called
#endif

    if (r3ACmd == ECmd_CameraPreviewStart)
    {
        //set 3A scheduler parameters
        //sync with MW:
        //if MinFps == MaxFps, MW query M with this fps; ==> here we use this fps to set M
        //otherwise, just normal preview case, MW won't query M; ==> here we use 30 fps to set M =1
        MUINT32 fps = (m_rParam.i4MinFps == m_rParam.i4MaxFps) ? m_rParam.i4MaxFps : 30000;
        mpScheduler->setSchdParams(m_i4SensorDev, fps, ((m_3ALogEnable & EN_3A_SCHEDULE_LOG) ? MTRUE : MFALSE));
    }
    if (r3ACmd == ECmd_CameraPreviewEnd)
    {
        //set default scheduling
        mpScheduler->setSchdParams(m_i4SensorDev, 30000, ((m_3ALogEnable & EN_3A_SCHEDULE_LOG) ? MTRUE : MFALSE));
    }
    if (r3ACmd == ECmd_CaptureEnd)
    {
        capCheckAndFireFlash_End();
    }
*/
    switch (r3ACmd){
        //type 1: run command by 3A thread
        case ECmd_CameraPreviewStart:
        //case ECmd_CaptureStart:
        //case ECmd_PrecaptureEnd:
            addCommandQ(r3ACmd);
            //::sem_wait(&mSem);
            bRet = sem_wait_relativeTime(&mSem, SEM_TIME_LIMIT_NS, "sendCommand::mSem(1)");
            if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(mSem(1)).");
        break;

        //type 2: without wait
        case ECmd_Update:
        //case ECmd_CaptureStart:
            addCommandQ(r3ACmd, i4Arg);
        break;

        //type 3: clear previous commands and run current command by 3A thread
        //case ECmd_RecordingStart: //from type1 to type3, avoid recording delay
        //case ECmd_RecordingEnd: //from type1 to type3, avoid recording stop delay
        case ECmd_CameraPreviewEnd:
        //case ECmd_CaptureEnd:
        case ECmd_Uninit:
            clearCommandQ();
            addCommandQ(r3ACmd);
            //::sem_wait(&mSem);
            bRet = sem_wait_relativeTime(&mSem, SEM_TIME_LIMIT_NS, "sendCommand::mSem(2)");
            if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(mSem(2)).");
        break;

        case ECmd_PrecaptureStart: //from type1 to type3, avoid capture delay
            clearCommandQ();
            addCommandQ(r3ACmd);
        break;

        //type 4: run current command directly by caller thread
        default:
            bRet = postCommand(r3ACmd, i4Arg);
            if (!bRet) AEE_ASSERT_3A_HAL("type4 postCommand fail.");
        break;
    }

    return true;
}

/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3AFlowCtrl::clearCommandQ()
{
    MY_LOG("[%s]+", __FUNCTION__);
    Mutex::Autolock autoLock(mModuleMtx);

    int Qsize = mCmdQ.size();

    for (List<CmdQ_T>::iterator it = mCmdQ.begin(); it != mCmdQ.end();)
    {
        ECmd_T eCmd = it->eCmd;
        if ((eCmd == ECmd_Update) && (mCmdQ.size() >= 2)) //add size limitation to avoid mCmdQ deadlock
        {
            MY_LOG("[%s] cmd size(%d), clear(%d)", __FUNCTION__, mCmdQ.size(), eCmd);
            it = mCmdQ.erase(it);
        }
        else //include 2 cases; 1). keep all cmds except for ECmd_Update, 2). keep at least 1 ECmd_Update in mCmdQ
        {
            MY_LOG("[%s] Command not killed: cmd size(%d), beginning cmd(%d)", __FUNCTION__, mCmdQ.size(), eCmd);
            it++;
        }
    }
    MY_LOG("[%s]- Qsize(%d)", __FUNCTION__, Qsize);
}

/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3AFlowCtrl::addCommandQ(ECmd_T const &r3ACmd, MINTPTR i4Arg)
{
    Mutex::Autolock autoLock(mModuleMtx);

    ParamIspProfile_T rParam;
    // data clone
    if (i4Arg != 0)
    {
        rParam = *reinterpret_cast<ParamIspProfile_T*>(i4Arg);
    }

    mCmdQ.push_back(CmdQ_T(r3ACmd, rParam));
    mCmdQCond.broadcast();

    MY_LOG_IF(m_3ALogEnable, "[%s] mCmdQ size(%d) + cmd(%d) magic(%d)", __FUNCTION__, mCmdQ.size(), r3ACmd, rParam.i4MagicNum);
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3AFlowCtrl::getCommand(CmdQ_T &rCmd, MBOOL &bGetCmd, MBOOL en_timeout)
{
    //returning MFALSE means break onThreadLoop
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] +", __FUNCTION__);
    Mutex::Autolock autoLock(mModuleMtx);
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] mCmdQ.size()=%d, en_timeout(%d)", __FUNCTION__, mCmdQ.size(), en_timeout);

    if (mCmdQ.size() == 0)
    {
        if (en_timeout)
        {
            MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] mCmdQCond.waitRelative +", __FUNCTION__);
            mCmdQCond.waitRelative(mModuleMtx, (long long int)27000000/*(mpScheduler->getVsTimeOutLimit_ns())*/);
            MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] mCmdQCond.waitRelative -", __FUNCTION__);
        }
        else
        {
            MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] mCmdQCond.wait +", __FUNCTION__);
            mCmdQCond.wait(mModuleMtx);
            MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] mCmdQCond.wait -", __FUNCTION__);
        }
    }
    if (mCmdQ.size() == 0) //this is only for en_timeout == 1 & timeout case
    {
        MY_LOG("[%s] mCmdQ.size() = %d after mCmdQCond.wait/waitRelative\n", __FUNCTION__, mCmdQ.size());
        bGetCmd = MFALSE; // timeout, no command received
        return MTRUE;
    }
    bGetCmd = MTRUE;
    // two cases: 1. en_timeout == 1, but get command in time
    //                 2. en_timeout == 0, finally get command

    rCmd = *mCmdQ.begin();
    MY_LOG_IF(m_3ALogEnable, "CmdQ-size(%d), todo(%d, %d)", mCmdQ.size(), rCmd.eCmd, rCmd.rParamIspProfile.i4MagicNum);

    mCmdQ.erase(mCmdQ.begin());

    if (rCmd.eCmd == ECmd_Uninit){
        return MFALSE;
    }
    else {
        return MTRUE;
    }
}
#if 0
/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3AFlowCtrl::waitVSirqThenGetCommand(CmdQ_T &rCmd)
{
    //returning MFALSE means break onThreadLoop
    MBOOL bRet;
    MBOOL bGetCmd;
    MINT32 time_ms;
    IEventIrq::Duration duration;

    if (!mbListenVsync) return getCommand(rCmd, bGetCmd, MFALSE);

    while(1)
    {
        /*temp mark out*///waitVSirq();
        mpIEventIrq->wait(duration);
        mpIEventIrq->mark();
        mpVSTimer->start("VS Timer", m_i4SensorDev, 1);
        bRet = getCommand(rCmd, bGetCmd, MTRUE);
        time_ms = mpVSTimer->End();

        if (bGetCmd) return bRet; //if receiving command, return
    }
    //impossible to get here, just to avoid compilation warning
    return MTRUE;

}
#endif
/******************************************************************************
*
*******************************************************************************/
MVOID*
Hal3AFlowCtrl::onThreadLoop(MVOID *arg)
{
    MY_LOG("+");

    CmdQ_T rCmd(ECmd_Update, ParamIspProfile_T());
    Hal3AFlowCtrl *_this = reinterpret_cast<Hal3AFlowCtrl*>(arg);
    AaaTimer VsyncTimer;
    MBOOL bGetCmd;
    IEventIrq::Duration duration;

    // (1) change thread setting
    _this->changeThreadSetting();

    // (2) thread-in-loop

    while (_this->getCommand(rCmd, bGetCmd, MFALSE))
    {
        //MY_LOG_IF(1, "rCmd(%d)+", rCmd);
        //
        switch (rCmd.eCmd)
        {
            case ECmd_Update:
            //case ECmd_CaptureStart:
                //
                if (! _this->mbListenUpdate) break;
                MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "rCmd(%d)+", rCmd.eCmd);
                //
                //::sem_post(&_this->mSem);
                //
                    //if ( 1/*! _this->isReadyToCapture()*/)
                    {
                        MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3AFlowCtrl::onThreadLoop] start waitVSirq.");
                        CAM_TRACE_FMT_BEGIN("3A_WaitVSync #(%d)", rCmd.rParamIspProfile.i4MagicNum);
                        _this->mpIEventIrq->wait(duration);
                        CAM_TRACE_FMT_END();
                        //_this->mpIEventIrq->mark();
                        VsyncTimer.start("VsyncUpdate", _this->m_i4SensorDev, ((_this->m_3ALogEnable & EN_3A_SCHEDULE_LOG) ? MTRUE : MFALSE));

                        if (_this->mpCbSet)
                            _this->mpCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_VSYNC_DONE, rCmd.rParamIspProfile.i4MagicNum, 0, 0);
                        if ( ! _this->postCommand(rCmd.eCmd, reinterpret_cast<MINTPTR>(&rCmd.rParamIspProfile)))
                        {
                            MY_ERR("Cmd(%d) failed(0x%x)", rCmd.eCmd, _this->getErrorCode());
                            AEE_ASSERT_3A_HAL("onThreadLoop ECmd_Update fail.");
                        }
                        VsyncTimer.End();
                    }

                //

                MY_LOG_IF(_this->m_3ALogEnable, "rCmd(%d), magic#(%d)-", rCmd.eCmd, rCmd.rParamIspProfile.i4MagicNum);
            break;

            default:
                MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "rCmd(%d)+", rCmd.eCmd);
                //
                if ( ! _this->postCommand(rCmd.eCmd, reinterpret_cast<MINTPTR>(&rCmd.rParamIspProfile)))
                {
                    MY_ERR("Cmd(%d) failed(0x%x)", rCmd.eCmd, _this->getErrorCode());
                    AEE_ASSERT_3A_HAL("onThreadLoop postCommand fail(2).");
                }
                //
                if (rCmd.eCmd != ECmd_PrecaptureStart)
                {
                    ::sem_post(&_this->mSem);
                }
                MY_LOG_IF(1, "rCmd(%d), magic#(%d)-", rCmd.eCmd, rCmd.rParamIspProfile.i4MagicNum);
            break;
        }
        //

    }

    MY_LOG("-");

    return NULL;
}





